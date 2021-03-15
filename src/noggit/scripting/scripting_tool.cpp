// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <cmath>

#include <das/Context.hpp>

#include <noggit/camera.hpp>
#include <noggit/Log.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_heap.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QCheckBox>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#define SCRIPT_FILE "script_settings.json"
#define INNER_RADIUS_PATH "__inner_radius"
#define OUTER_RADIUS_PATH "__outer_radius"
#define CUR_PROFILE_PATH "__cur_profile"

namespace noggit
{
  namespace scripting
  {
    template <typename T>
    static T get_json_safe (nlohmann::json& json, Loader const& loader, std::string const& cur_profile, std::string key, T def)
    {
      // TODO: this is terrible but accessing by reference didn't seem to work.
      auto ssn = loader.selected_script_name();

      if (!json[ssn][cur_profile].contains(key))
      {
        json[ssn][cur_profile][key] = def;
      }

      return json[ssn][cur_profile][key];
    }

    template <typename T>
    static void set_json_safe (nlohmann::json& json, Loader const& loader, std::string const& cur_profile, std::string key, T def)
    {
      auto ssn = loader.selected_script_name();
      json[ssn][cur_profile][key] = def;
    }

    template <typename T>
    static void set_json_unsafe (nlohmann::json& json, Loader const& loader, std::string const& cur_profile, std::string key, T value)
    {
      json[loader.selected_script_name()][cur_profile][key] = value;
    }

    template <typename T>
    static T get_json_unsafe (nlohmann::json& json, Loader const& loader, std::string const& cur_profile, std::string key)
    {
      return json[loader.selected_script_name()][cur_profile][key].get<T>();
    }

    void scripting_tool::doReload()
    {
      int selection = -1;
      removeScriptWidgets();
      clearLog();
      try
      {
        selection = _loader.load_scripts (this);
      }
      catch (std::exception const& e)
      {
        addLog("[error]: " + std::string(e.what()));
        resetLogScroll();
        return;
      }
      _selection->clear();

      for (int i = 0; i < _loader.script_count(); ++i)
      {
        _selection->addItem(_loader.get_script_display_name(i));
      }

      if (selection != -1)
      {
        _selection->setCurrentIndex(selection);
        on_change_script(selection);
      }
    }

    void scripting_tool::on_change_script(int selection)
    {
      std::lock_guard<std::mutex> const lock (_script_change_mutex);
      removeScriptWidgets();
      clearDescription();

      auto sn = _loader.get_script_name(selection);
      _profile_selection->clear();
      if (_json.contains(sn))
      {
        std::vector<std::string> items;
        for (auto& v : _json[sn].items())
        {
          if (v.key() != CUR_PROFILE_PATH)
          {
            items.push_back(v.key());
          }
        }

        std::sort(items.begin(), items.end(), [](auto a, auto b) {
          if (a == "Default")
            return true;
          if (b == "Default")
            return false;
          return a < b;
        });

        for (auto& item : items)
        {
          _profile_selection->addItem(QString::fromStdString (item));
        }
      }

      if (_profile_selection->count() == 0)
      {
        _profile_selection->addItem("Default");
      }

      int next_profile = 0;
      auto cur_script = _loader.get_script_name(selection);
      if (_json.contains(cur_script))
      {
        if (_json[cur_script].contains(CUR_PROFILE_PATH))
        {
          auto str = _json[cur_script][CUR_PROFILE_PATH];
          for (int i = 0; i < _profile_selection->count(); ++i)
          {
            if (_profile_selection->itemText(i) == QString::fromStdString (str))
            {
              next_profile = i;
              break;
            }
          }
        }
      }

      _cur_profile = _profile_selection->itemText(next_profile).toStdString();
      _profile_selection->setCurrentIndex(next_profile);
      _json[cur_script][CUR_PROFILE_PATH] = _cur_profile;
      _loader.select_script(selection, this);

      initialize_radius();
      for (auto& item : _string_arrays)
      {
        // if the index is 0, that MIGHT mean the value we have
        // is invalid (old script), so we just write it again to be safe.
        if (item.second->currentIndex() == 0)
        {
          set_json_unsafe (_json, _loader, _cur_profile, item.first, item.second->itemText(0).toStdString());
        }
      }
    }

    namespace
    {
      void readScriptSettings (nlohmann::json& json)
      {
        if (!boost::filesystem::exists(SCRIPT_FILE))
        {
          return;
        }

        try
        {
          std::ifstream(SCRIPT_FILE) >> json;
        }
        catch (std::exception err)
        {
          if(!boost::filesystem::exists(SCRIPT_FILE))
          {
            return;
          }
          // back up broken script settings, since they won't be read and will be overwritten.
          std::string backup_file = std::string(SCRIPT_FILE) + ".backup";
          int i = 0;
          while (boost::filesystem::exists(backup_file+std::to_string(i)))
          {
              ++i;
          }
          boost::filesystem::copy(SCRIPT_FILE, backup_file + std::to_string(i));

          // Add a message box here
        }
      }
    }

    scripting_tool::scripting_tool(QWidget* parent)
      : QWidget(parent)
      , _cur_profile ("Default")
    {
      readScriptSettings (_json);

      auto layout(new QVBoxLayout(this));
      _selection = new QComboBox();
      layout->addWidget(_selection);

      _reload_button = new QPushButton("Reload Scripts", this);
      layout->addWidget(_reload_button);
      connect(_reload_button, &QPushButton::released, this, [this]() {
        doReload();
      });

      // Profiles
      _profile_group = new QGroupBox("Script Profiles");
      _profile_select_column = new QGridLayout(_profile_group);

      _profile_selection = new QComboBox(this);
      _profile_selection->addItem("Default");
      _profile_remove_button = new QPushButton("X", this);
      _profile_name_entry = new QLineEdit(this);
      _profile_create_button = new QPushButton("Add", this);

      _profile_select_column->addWidget(_profile_selection, 0, 0);
      _profile_select_column->addWidget(_profile_remove_button, 0, 1);

      _profile_select_column->addWidget(_profile_name_entry, 1, 0);
      _profile_select_column->addWidget(_profile_create_button, 1, 1);

      layout->addWidget(_profile_group);

      _radius_spin = new QDoubleSpinBox(this);
      _radius_spin->setRange(0.0f, 1000.0f);
      _radius_spin->setDecimals(2);
      _radius_spin->setValue(_radius);

      _radius_slider = new QSlider(Qt::Orientation::Horizontal, this);
      _radius_slider->setRange(0, 1000);
      _radius_slider->setSliderPosition((int)std::round(_radius));

      _inner_radius_spin = new QDoubleSpinBox(this);
      _inner_radius_spin->setRange(0.0f, 1.0f);
      _inner_radius_spin->setDecimals(2);
      _inner_radius_spin->setValue(_inner_radius);
      _inner_radius_spin->setSingleStep(0.05f);

      _inner_radius_slider = new QSlider(Qt::Orientation::Horizontal, this);
      _inner_radius_slider->setRange(0, 100);
      _inner_radius_slider->setSliderPosition((int)std::round(_inner_radius * 100));

      _radius_group = new QGroupBox("Radius");
      _radius_layout = new QFormLayout(_radius_group);
      _radius_layout->addRow("Outer:", _radius_spin);
      _radius_layout->addRow(_radius_slider);
      _radius_layout->addRow("Inner:", _inner_radius_spin);
      _radius_layout->addRow(_inner_radius_slider);

      layout->addWidget(_radius_group);

      _script_settings_group = new QGroupBox("Script Settings");
      _script_settings_layout = new QFormLayout(_script_settings_group);
      layout->addWidget(_script_settings_group);

      _description = new QLabel(this);
      layout->addWidget(_description);

      _log = new QPlainTextEdit(this);
      _log->setFont (QFontDatabase::systemFont (QFontDatabase::FixedFont));
      _log->setReadOnly(true);
      layout->addWidget(_log);

      connect(_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_slider);
        _radius_slider->setSliderPosition((int)std::round(v));
        set_json_unsafe (_json, _loader, _cur_profile, OUTER_RADIUS_PATH, v);
      });

      connect(_radius_slider, &QSlider::valueChanged, [&](int v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_spin);
        _radius_spin->setValue(v);
        set_json_unsafe (_json, _loader, _cur_profile, OUTER_RADIUS_PATH, v);
      });

      connect(_inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _inner_radius = v;
        QSignalBlocker const blocker(_inner_radius_slider);
        _inner_radius_slider->setSliderPosition((int)std::round(v * 100));
        set_json_unsafe (_json, _loader, _cur_profile, INNER_RADIUS_PATH, v);
      });

      connect(_inner_radius_slider, &QSlider::valueChanged, [&](int v) {
        _inner_radius = v / 100.0f;
        QSignalBlocker const blocker(_inner_radius_spin);
        _inner_radius_spin->setValue(_inner_radius);
        set_json_unsafe (_json, _loader, _cur_profile, INNER_RADIUS_PATH, _inner_radius);
      });

      connect(_profile_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        select_profile(index);
      });

      connect(_profile_remove_button, &QPushButton::released, this, [this]() {
        auto script_name = _loader.selected_script_name();
        if (script_name.size() == 0)
        {
          // TODO: error?
          return;
        }

        auto index = _profile_selection->currentIndex();

        // do not allow deleting default settings
        if (index == 0)
        {
          return;
        }

        auto text = _profile_selection->itemText(index).toStdString();
        _profile_selection->removeItem(index);

        if (_json.contains(script_name))
        {
          if (_json[script_name].contains(text))
          {
            _json[script_name].erase(text);
          }
        }

        // go back to default, so we don't accidentally delete another profile
        select_profile(0);
      });

      connect(_profile_create_button, &QPushButton::released, this, [this]() {
        auto script_name = _loader.selected_script_name();

        // do not allow invalid script
        if (script_name.size() == 0)
          return;

        auto newText = _profile_name_entry->text();

        // do not allow empty profiles
        if (newText.isEmpty())
          return;

        auto count = _profile_selection->count();
        for (int i = 0; i < count; ++i)
        {
          // do not allow duplicate profiles
          if (_profile_selection->itemText(i) == newText)
          {
            return;
          }
        }

        _profile_name_entry->clear();
        _profile_selection->addItem (newText);

        if (!_json[script_name].contains(newText.toStdString()))
        {
          if (_json[script_name].contains(_cur_profile))
          {
            _json[script_name][newText.toStdString()] = _json[script_name][_cur_profile];
          }
        }

        _profile_selection->setCurrentIndex(count);

        select_profile(count);
      });

      connect(_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        clearLog();
        on_change_script(index);
      });

      doReload();
    }

    scripting_tool::~scripting_tool()
    {
      save_json();
      }

    void scripting_tool::save_json() const
      {
      std::ofstream(SCRIPT_FILE) << std::setw(4) << _json << "\n";
    }

// i don't remember why this was a macro
#define ADD_SLIDER(path, T, min, max, def, decimals)                                 \
  double dp1 = decimals > 0 ? decimals + 5 : decimals + 1;                           \
  auto spinner = new QDoubleSpinBox(this);                                           \
  spinner->setRange(min, max);                                                       \
  spinner->setDecimals(decimals);                                                    \
  spinner->setValue(def);                                                            \
  auto slider = new QSlider(Qt::Orientation::Horizontal, this);                      \
  slider->setRange(min * dp1, max * dp1);                                            \
  slider->setSliderPosition((int)std::round(def * dp1));                             \
  auto label = new QLabel(this);                                                     \
  label->setText(name);                                                              \
  connect(spinner, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double v) { \
    set_json_unsafe<T> (_json, _loader, _cur_profile, path, (T)v);                                                  \
    QSignalBlocker const blocker(slider);                                            \
    slider->setSliderPosition((int)std::round(v * dp1));                             \
  });                                                                                \
  connect(slider, &QSlider::valueChanged, [=](int v) {                               \
    double t = double(v) / dp1;                                                      \
    set_json_unsafe<T> (_json, _loader, _cur_profile, path, t);                                                     \
    QSignalBlocker const blocker(spinner);                                           \
    spinner->setValue(t);                                                            \
  });                                                                                \
                                                                                     \
  _script_settings_layout->addRow(label, spinner);                                   \
  _script_settings_layout->addRow("", slider);                                       \
  _script_widgets.push_back(label);                                                  \
  _script_widgets.push_back(spinner);                                                \
  _script_widgets.push_back(slider);                                                 \
  set_json_safe<T> (_json, _loader, _cur_profile, path, std::min(max, std::max(min, get_json_safe<T> (_json, _loader, _cur_profile, path, def)))); \
  auto v = get_json_safe<T> (_json, _loader, _cur_profile, path, def);                                              \
  slider->setSliderPosition((int)std::round(v * dp1));                               \
  spinner->setValue(v);

    void scripting_tool::addDouble(char const* name, double min, double max, double def, int zeros)
    {
      ADD_SLIDER(name, double, min, max, def, zeros);
    }

    void scripting_tool::addInt(char const* name, int min, int max, int def)
    {
      ADD_SLIDER(name, int, min, max, def, 0);
    }

    void scripting_tool::addBool(char const* name, bool def)
    {
      auto checkbox = new QCheckBox(this);
      auto label = new QLabel(this);
      label->setText(name);
      connect(checkbox, &QCheckBox::stateChanged, this, [=](auto value) {
        set_json_unsafe<bool> (_json, _loader, _cur_profile, name, value ? true : false);
      });

      _script_widgets.push_back(checkbox);
      _script_widgets.push_back(label);
      _script_settings_layout->addRow(label, checkbox);

      checkbox->setCheckState(get_json_safe<bool> (_json, _loader, _cur_profile, name, def) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }

    void scripting_tool::addStringList(char const* name, char const* value)
    {

      if (_string_arrays.find(name) == _string_arrays.end())
      {
        auto box = new QComboBox(this);
        auto label = new QLabel(this);

        connect(box, QOverload<int>::of(&QComboBox::activated), this, [=](auto index) {
          set_json_unsafe (_json, _loader, _cur_profile, name, box->itemText(index).toStdString());
        });

        box->addItem(value);
        label->setText(name);

        _string_arrays[name] = box;
        _script_widgets.push_back(box);
        _script_widgets.push_back(label);
        _script_settings_layout->addRow(label, box);

        // ensure there is at least one valid value in it
        get_json_safe<std::string> (_json, _loader, _cur_profile, name, value);
      }
      else
      {
        auto box = _string_arrays[name];
        box->addItem(value);

        // we found the last selection, so change the index for that.
        if (get_json_safe<std::string> (_json, _loader, _cur_profile, name, "") == value)
        {
          box->setCurrentIndex(box->count() - 1);
        }
      }
    }

    void scripting_tool::addString(char const* name, char const* def)
    {
      auto defstr = def == nullptr ? "" : def;
      auto tline = new QLineEdit(this);
      auto label = new QLabel(this);
      label->setText(name);
      connect(tline, &QLineEdit::textChanged, this, [=](auto text) {
        set_json_unsafe (_json, _loader, _cur_profile, name, text.toStdString());
      });
      _script_widgets.push_back(label);
      _script_widgets.push_back(tline);
      _script_settings_layout->addRow(label, tline);
      tline->setText (QString::fromStdString (get_json_safe<std::string> (_json, _loader, _cur_profile, name, defstr)));
    }

    void scripting_tool::removeScriptWidgets()
    {
      for (auto& widget : _script_widgets)
      {
        _script_settings_layout->removeWidget(widget);
        delete widget;
      }

      _script_widgets.clear();
      _string_arrays.clear();
    }

    void scripting_tool::select_profile(int profile)
    {
      removeScriptWidgets();
      clearDescription();
      _cur_profile = _profile_selection->itemText(profile).toStdString();

      auto n = _loader.selected_script_name();
      _json[n][CUR_PROFILE_PATH] = _cur_profile;

      initialize_radius();

      _loader.select_script (_loader.get_selected_script(), this);
    }

    void scripting_tool::sendUpdate(
        World* world,
        math::vector_3d pos_in,
        noggit::camera* cam,
        float dt,
        bool left_mouse,
        bool right_mouse,
        bool holding_shift,
        bool holding_ctrl,
        bool holding_alt,
        bool holding_space)
    {
      script_context ctx(world, pos_in, brushRadius(), innerRadius(), cam, holding_alt, holding_shift, holding_ctrl, holding_space, dt);
      set_ctx(&ctx);

      try
      {
        if (left_mouse)
        {
          if (!_last_left)
          {
            _loader.send_left_click (this);
          }

          _loader.send_left_hold (this);
        }

        if (right_mouse)
        {
          if (!_last_right)
          {
            _loader.send_right_click (this);
          }
          _loader.send_right_hold (this);
        }

        if (!left_mouse && _last_left)
        {
          _loader.send_left_release (this);
        }

        if (!right_mouse && _last_right)
        {
          _loader.send_right_release (this);
        }
      }
      catch (std::exception const& e)
      {
        doReload();
        addLog(("[error]: " + std::string(e.what())));
        resetLogScroll();
      }

      script_free_all();
      set_ctx(nullptr);
      _last_left = left_mouse;
      _last_right = right_mouse;
    }

    void scripting_tool::initialize_radius()
    {
      double inner_radius = get_json_safe<double> (_json, _loader, _cur_profile, INNER_RADIUS_PATH, 0.5);
      double outer_radius = get_json_safe<double> (_json, _loader, _cur_profile, OUTER_RADIUS_PATH, 40);

      _radius_spin->setValue(outer_radius);
      _inner_radius_spin->setValue(inner_radius);

      _radius_slider->setValue(std::round(outer_radius));
      _inner_radius_slider->setSliderPosition((int)std::round(inner_radius*100));
    }

    void scripting_tool::addDescription(char const* text)
    {
      std::string stext = text == nullptr ? "" : text;
      _description->setText(_description->text() + "\n" + QString::fromStdString (stext));
    }

    void scripting_tool::addLog(std::string const& text)
    {
      LogDebug << "[script window]: " << text << "\n";
      _log->appendPlainText (QString::fromStdString (text));
      _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
    }

    void scripting_tool::resetLogScroll()
    {
      _log->verticalScrollBar()->setValue(0);
    }

    void scripting_tool::clearLog()
    {
      _log->clear();
    }

    void scripting_tool::clearDescription()
    {
      _description->clear();
    }

    char const* get_string_param(char const* path, das::Context* context)
    {
      return script_calloc_string
        ( get_json_unsafe<std::string>
            ( static_cast<Loader::Context*> (context)->_tool->_json
            , static_cast<Loader::Context*> (context)->_tool->_loader
            , static_cast<Loader::Context*> (context)->_tool->_cur_profile
            , path
            )
        , context
        );
    }

    char const* get_string_list_param(char const* path, das::Context* context)
    {
      return get_string_param(path, context);
    }

    int get_int_param(char const* path, das::Context* context)
    {
      return get_json_unsafe<int>
        ( static_cast<Loader::Context*> (context)->_tool->_json
        , static_cast<Loader::Context*> (context)->_tool->_loader
        , static_cast<Loader::Context*> (context)->_tool->_cur_profile
        , path
        );
    }

    double get_double_param(char const* path, das::Context* context)
    {
      return get_json_unsafe<double>
        ( static_cast<Loader::Context*> (context)->_tool->_json
        , static_cast<Loader::Context*> (context)->_tool->_loader
        , static_cast<Loader::Context*> (context)->_tool->_cur_profile
        , path
        );
    }

    float get_float_param(char const* path, das::Context* context)
    {
      return get_json_unsafe<double>
        ( static_cast<Loader::Context*> (context)->_tool->_json
        , static_cast<Loader::Context*> (context)->_tool->_loader
        , static_cast<Loader::Context*> (context)->_tool->_cur_profile
        , path
        );
    }

    bool get_bool_param(char const* path, das::Context* context)
    {
      return get_json_unsafe<bool>
        ( static_cast<Loader::Context*> (context)->_tool->_json
        , static_cast<Loader::Context*> (context)->_tool->_loader
        , static_cast<Loader::Context*> (context)->_tool->_cur_profile
        , path
        );
    }

    void add_string_param(char const* path, char const* def, das::Context* context)
    {
      if(path==nullptr)
      {
        throw script_exception(
          "add_string_param",
          std::string("empty name to string list parameter, default value =")
          + std::string(def==nullptr ? "null" : def)
        );
      }
      static_cast<Loader::Context*> (context)->_tool->addString(path, def != nullptr ? def : "");
    }

    void add_int_param(char const* path, int min, int max, int def, das::Context* context)
    {
      if(path==nullptr)
      {
        throw script_exception(
          "add_int_param",
          std::string("empty path to int parameter")
        );
      }
      static_cast<Loader::Context*> (context)->_tool->addInt(path, min, max, def);
    }

    void add_double_param(char const* path, double min, double max, double def, int zeros, das::Context* context)
    {
      if(path==nullptr)
      {
        throw script_exception(
          "add_double_param",
          std::string("empty path to double parameter")
        );
      }
      static_cast<Loader::Context*> (context)->_tool->addDouble(path, min, max, def, zeros);
    }

    void add_float_param(char const* path, float min, float max, float def, int zeros, das::Context* context)
    {
      if(path==nullptr)
      {
        throw script_exception(
          "add_float_param",
          std::string("empty path to float parameter")
        );
      }
      static_cast<Loader::Context*> (context)->_tool->addDouble(path, min, max, def, zeros);
    }

    void add_bool_param(char const* path, bool def, das::Context* context)
    {
      if(path==nullptr)
      {
        throw script_exception(
          "add_bool_param",
          std::string("empty path to bool parameter")
        );
      }
      static_cast<Loader::Context*> (context)->_tool->addBool(path, def);
    }

    void add_description(char const* path, das::Context* context)
    {
      static_cast<Loader::Context*> (context)->_tool->addDescription(path != nullptr ? path : "");
    }

    void add_string_list_param(char const* path, char const* value, das::Context* context)
    {
      if (path == nullptr)
      {
          throw script_exception(
              "add_string_list_param",
              std::string("empty name to string list parameter, default value =")
              + std::string(value == nullptr ? "" : value)
          );
      }
      if (value == nullptr) throw script_exception(
          "add_string_list_param",
          std::string("empty value to string list parameter ")
          + path
      );
      static_cast<Loader::Context*> (context)->_tool->addStringList(path, value);
    }
  } // namespace scripting
} // namespace noggit
