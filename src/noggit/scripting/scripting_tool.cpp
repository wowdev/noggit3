// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <cmath>

#include <noggit/camera.hpp>
#include <noggit/Log.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/scripting/script_heap.hpp>

#include <json.hpp>
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

using json = nlohmann::json;
namespace noggit
{
  namespace scripting
  {

    static json _json;
    static std::vector<char *> strings;
    static std::string cur_profile = "Default";
    static boost::mutex script_change_mutex;

    template <typename T>
    static T get_json_safe(std::string key, T def)
    {
      // TODO: this is terrible but accessing by reference didn't seem to work.
      auto ssn = selected_script_name();
      if (!_json.contains(ssn))
      {
        _json[ssn] = json();
      }

      if (!_json[ssn].contains(cur_profile))
      {
        _json[ssn][cur_profile] = json();
      }

      if (!_json[ssn][cur_profile].contains(key))
      {
        _json[ssn][cur_profile][key] = def;
      }

      return _json[ssn][cur_profile][key];
    }

    template <typename T>
    static void set_json_safe(std::string key, T def)
    {
      auto ssn = selected_script_name();
      if (!_json.contains(ssn))
      {
        _json[ssn] = json();
      }

      if (!_json[ssn].contains(cur_profile))
      {
        _json[ssn][cur_profile] = json();
      }

      _json[ssn][cur_profile][key] = def;
    }

    template <typename T>
    static void set_json_unsafe(std::string key, T value)
    {
      _json[selected_script_name()][cur_profile][key] = value;
    }

    template <typename T>
    static T get_json_unsafe(std::string key)
    {
      return _json[selected_script_name()][cur_profile][key].get<T>();
    }

    static scripting_tool *cur_tool = nullptr;

    scripting_tool *get_cur_tool()
    {
      return cur_tool;
    }

    void scripting_tool::doReload()
    {
      cur_tool = this;
      int selection = -1;
      removeScriptWidgets();
      clearLog();
      try
      {
        selection = load_scripts();
      }
      catch (std::exception e)
      {
        addLog("[error]: " + std::string(e.what()));
        return;
      }
      _script_selection->clear();

      for (int i = 0; i < script_count(); ++i)
      {
        _script_selection->addItem(get_script_display_name(i).c_str());
      }

      if (selection != -1)
      {
        _script_selection->setCurrentIndex(selection);
        on_change_script(selection);
      }
    }

    void scripting_tool::on_change_script(int selection)
    {
      script_change_mutex.lock();
      removeScriptWidgets();
      clearDescription();

      cur_tool = this;

      auto sn = get_script_name(selection);
      _profile_selection->clear();
      if (_json.contains(sn))
      {
        std::vector<std::string> items;
        for (auto &v : _json[sn].items())
        {
          if (v.key() != CUR_PROFILE_PATH)
          {
            items.push_back(v.key().c_str());
          }
        }

        std::sort(items.begin(), items.end(), [](auto a, auto b) {
          if (a == "Default")
            return true;
          if (b == "Default")
            return false;
          return a < b;
        });

        for (auto &item : items)
        {
          _profile_selection->addItem(item.c_str());
        }
      }

      if (_profile_selection->count() == 0)
      {
        _profile_selection->addItem("Default");
      }

      int next_profile = 0;
      auto cur_script = get_script_name(selection);
      if (_json.contains(cur_script))
      {
        if (_json[cur_script].contains(CUR_PROFILE_PATH))
        {
          auto str = _json[cur_script][CUR_PROFILE_PATH];
          for (int i = 0; i < _profile_selection->count(); ++i)
          {
            if (_profile_selection->itemText(i).toUtf8().constData() == str)
            {
              next_profile = i;
              break;
            }
          }
        }
      }

      cur_profile = _profile_selection->itemText(next_profile).toUtf8().constData();
      _profile_selection->setCurrentIndex(next_profile);
      if (!_json.contains(cur_script))
      {
        _json[cur_script] = json();
      }
      _json[cur_script][CUR_PROFILE_PATH] = cur_profile;
      select_script(selection);

      initialize_radius();
      for (auto &item : _string_arrays)
      {
        // if the index is 0, that MIGHT mean the value we have
        // is invalid (old script), so we just write it again to be safe.
        if (item.second->currentIndex() == 0)
        {
          set_json_unsafe(item.first, item.second->itemText(0).toUtf8().constData());
        }
      }

      cur_tool = nullptr;
      script_change_mutex.unlock();
    }

    scripting_tool::scripting_tool(QWidget *parent) : QWidget(parent)
    {
      auto layout(new QFormLayout(this));
      _script_selection = new QComboBox();
      layout->addRow(_script_selection);

      _reload_button = new QPushButton("Reload Scripts", this);
      layout->addRow(_reload_button);
      connect(_reload_button, &QPushButton::released, this, [this]() {
        cur_tool = this;
        doReload();
        cur_tool = nullptr;
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

      layout->addRow(_profile_group);

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
      _log->setReadOnly(true);
      layout->addWidget(_log);

      connect(_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_slider);
        _radius_slider->setSliderPosition((int)std::round(v));
        set_json_unsafe(OUTER_RADIUS_PATH, v);
      });

      connect(_radius_slider, &QSlider::valueChanged, [&](int v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_spin);
        _radius_spin->setValue(v);
        set_json_unsafe(OUTER_RADIUS_PATH, v);
      });

      connect(_inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _inner_radius = v;
        QSignalBlocker const blocker(_inner_radius_slider);
        _inner_radius_slider->setSliderPosition((int)std::round(v * 100));
        set_json_unsafe(INNER_RADIUS_PATH, v);
      });

      connect(_inner_radius_slider, &QSlider::valueChanged, [&](int v) {
        _inner_radius = v / 100.0f;
        QSignalBlocker const blocker(_inner_radius_spin);
        _inner_radius_spin->setValue(_inner_radius);
        set_json_unsafe(INNER_RADIUS_PATH, v);
      });

      connect(_profile_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        select_profile(index);
      });

      connect(_profile_remove_button, &QPushButton::released, this, [this]() {
        auto script_name = selected_script_name();
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

        auto text = _profile_selection->itemText(index).toUtf8().constData();
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
        auto script_name = selected_script_name();

        // do not allow invalid script
        if (script_name.size() == 0)
          return;

        std::string newText = _profile_name_entry->text().toUtf8().constData();

        // do not allow empty profiles
        if (newText.size() == 0)
          return;

        auto count = _profile_selection->count();
        for (int i = 0; i < count; ++i)
        {
          // do not allow duplicate profiles
          if (_profile_selection->itemText(i).toUtf8().constData() == newText)
          {
            return;
          }
        }

        _profile_name_entry->clear();
        _profile_selection->addItem(newText.c_str());
        if (!_json.contains(script_name))
        {
          _json[script_name] = json();
        }

        if (!_json[script_name].contains(newText))
        {
          if (_json[script_name].contains(cur_profile))
          {
            _json[script_name][newText] = _json[script_name][cur_profile];
          }
          else
          {
            _json[script_name][newText] = json();
          }
        }

        _profile_selection->setCurrentIndex(count);

        select_profile(count);
      });

      connect(_script_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        clearLog();
        on_change_script(index);
      });

      if (boost::filesystem::exists(SCRIPT_FILE))
      {
        std::ifstream(SCRIPT_FILE) >> _json;
      }
      doReload();
    }

// i don't remember why this was a macro
#define ADD_SLIDER(path, T, min, max, def, decimals)                                 \
  double dp1 = decimals > 0 ? decimals + 5 : decimals + 1;                           \
  auto spinner = new QDoubleSpinBox(this);                                           \
  spinner->setRange(min, max);                                                       \
  spinner->setDecimals(decimals);                                                    \
  spinner->setValue(def);                                                            \
  auto slider = new QSlider(Qt::Orientation::Horizontal, this);                      \
  slider->setRange(min *dp1, max *dp1);                                              \
  slider->setSliderPosition((int)std::round(def *dp1));                              \
  auto label = new QLabel(this);                                                     \
  label->setText(name);                                                              \
  connect(spinner, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double v) { \
    set_json_unsafe<T>(path, (T)v);                                                  \
    QSignalBlocker const blocker(slider);                                            \
    slider->setSliderPosition((int)std::round(v *dp1));                              \
  });                                                                                \
  connect(slider, &QSlider::valueChanged, [=](int v) {                               \
    double t = double(v) / dp1;                                                      \
    set_json_unsafe<T>(path, t);                                                     \
    QSignalBlocker const blocker(spinner);                                           \
    spinner->setValue(t);                                                            \
  });                                                                                \
                                                                                     \
  _script_settings_layout->addRow(label, spinner);                                   \
  _script_settings_layout->addRow("", slider);                                       \
  _script_widgets.push_back(label);                                                  \
  _script_widgets.push_back(spinner);                                                \
  _script_widgets.push_back(slider);                                                 \
  set_json_safe<T>(path, std::min(max, std::max(min, get_json_safe<T>(path, def)))); \
  auto v = get_json_safe<T>(path, def);                                              \
  slider->setSliderPosition((int)std::round(v *dp1));                                \
  spinner->setValue(v);

    void scripting_tool::addDouble(const char *name, double min, double max, double def, int zeros)
    {
      ADD_SLIDER(name, double, min, max, def, zeros);
    }

    void scripting_tool::addInt(const char *name, int min, int max, int def)
    {
      ADD_SLIDER(name, int, min, max, def, 0);
    }

    void scripting_tool::addBool(const char *name, bool def)
    {
      auto checkbox = new QCheckBox(this);
      auto label = new QLabel(this);
      label->setText(name);
      connect(checkbox, &QCheckBox::stateChanged, this, [=](auto value) {
        set_json_unsafe<bool>(name, value ? true : false);
      });

      _script_widgets.push_back(checkbox);
      _script_widgets.push_back(label);
      _script_settings_layout->addRow(label, checkbox);

      checkbox->setCheckState(get_json_safe<bool>(name, def) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }

    void scripting_tool::addStringList(const char *name, const char *value)
    {

      if (_string_arrays.find(name) == _string_arrays.end())
      {
        auto layout = new QFormLayout(this);
        auto box = new QComboBox(this);
        auto label = new QLabel(this);

        connect(box, QOverload<int>::of(&QComboBox::activated), this, [=](auto index) {
          set_json_unsafe<std::string>(name, box->itemText(index).toUtf8().constData());
        });

        box->addItem(value);
        label->setText(name);

        _string_arrays[name] = box;
        _script_widgets.push_back(box);
        _script_widgets.push_back(label);
        _script_settings_layout->addRow(label, box);

        // ensure there is at least one valid value in it
        get_json_safe<std::string>(name, value);
      }
      else
      {
        auto box = _string_arrays[name];
        box->addItem(value);

        // we found the last selection, so change the index for that.
        if (get_json_safe<std::string>(name, "") == value)
        {
          box->setCurrentIndex(box->count() - 1);
        }
      }
    }

    void scripting_tool::addString(const char *name, const char *def)
    {
      auto defstr = def == nullptr ? "" : def;
      auto tline = new QLineEdit(this);
      auto label = new QLabel(this);
      label->setText(name);
      connect(tline, &QLineEdit::textChanged, this, [=](auto text) {
        set_json_unsafe<std::string>(name, text.toUtf8().constData());
      });
      _script_widgets.push_back(label);
      _script_widgets.push_back(tline);
      _script_settings_layout->addRow(label, tline);
      tline->setText(get_json_safe<std::string>(name, defstr).c_str());
    }

    void scripting_tool::removeScriptWidgets()
    {
      for (auto &widget : _script_widgets)
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
      cur_tool = this;
      cur_profile = _profile_selection->itemText(profile).toUtf8().constData();

      auto n = selected_script_name();
      if (!_json.contains(n))
      {
        _json[n] = json();
      }

      _json[n][CUR_PROFILE_PATH] = cur_profile;

      initialize_radius();

      select_script(get_selected_script());
      cur_tool = nullptr;
    }

    void scripting_tool::sendUpdate(
        World *world,
        math::vector_3d pos_in,
        noggit::camera *cam,
        float dt,
        bool left_mouse,
        bool right_mouse,
        bool holding_shift,
        bool holding_ctrl,
        bool holding_alt,
        bool holding_space)
    {
      cur_tool = this;
      script_context ctx(world, pos_in, brushRadius(), innerRadius(), cam, holding_alt, holding_shift, holding_ctrl, holding_space);
      set_ctx(&ctx);

      try
      {
        if (left_mouse)
        {
          if (!_last_left)
          {
            send_left_click();
          }

          send_left_hold();
        }

        if (right_mouse)
        {
          if (!_last_right)
          {
            send_right_click();
          }
          send_right_hold();
        }

        if (!left_mouse && _last_left)
        {
          send_left_release();
        }

        if (!right_mouse && _last_right)
        {
          send_right_release();
        }
      }
      catch (const std::exception &e)
      {
        addLog(("[error]: " + std::string(e.what())));
      }

      script_free_all();
      set_ctx(nullptr);
      cur_tool = nullptr;
      _last_left = left_mouse;
      _last_right = right_mouse;
    }

    void scripting_tool::initialize_radius()
    {
      double inner_radius = get_json_safe<double>(INNER_RADIUS_PATH, 0);
      double outer_radius = get_json_safe<double>(OUTER_RADIUS_PATH, 0);

      _radius_spin->setValue(outer_radius);
      _inner_radius_spin->setValue(inner_radius);

      _radius_slider->setValue(std::round(outer_radius));
      _inner_radius_slider->setValue(std::round(inner_radius));
    }

    void scripting_tool::addDescription(const char *text)
    {
      std::string stext = text == nullptr ? "" : text;
      _description->setText(_description->text() + "\n" + stext.c_str());
    }

    void scripting_tool::addLog(const std::string &text)
    {
      LogDebug << "[script window]: " << text << "\n";
      _log->appendPlainText(text.c_str());
      _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
    }

    void scripting_tool::clearLog()
    {
      _log->clear();
    }

    void scripting_tool::clearDescription()
    {
      _description->clear();
    }

    const char *get_string_param(const char *path)
    {
      auto str = get_json_unsafe<std::string>(path);

      // HACK: daScript somehow doesn't like receiving qt or json strings
      // idk if it's because they're on the stack or whatever
      // even copying it to another std::string doesn't work, but this does.
      char *chr = (char *)script_malloc(sizeof(char) * (str.size() + 1));
      str.copy(chr, str.size());
      chr[str.size()] = 0;
      return chr;
    }

    const char *get_string_list_param(const char *path)
    {
      return get_string_param(path);
    }

    int get_int_param(const char *path)
    {
      return get_json_unsafe<int>(path);
    }

    double get_double_param(const char *path)
    {
      return get_json_unsafe<double>(path);
    }

    float get_float_param(const char *path)
    {
      return get_json_unsafe<double>(path);
    }

    bool get_bool_param(const char *path)
    {
      return get_json_unsafe<bool>(path);
    }

    void add_string_param(const char *path, const char *def)
    {
      get_cur_tool()->addString(path, def);
    }

    void add_int_param(const char *path, int min, int max, int def)
    {
      get_cur_tool()->addInt(path, min, max, def);
    }

    void add_double_param(const char *path, double min, double max, double def, int zeros = 2)
    {
      get_cur_tool()->addDouble(path, min, max, def, zeros);
    }

    void add_float_param(const char *path, float min, float max, float def, int zeros)
    {
      get_cur_tool()->addDouble(path, min, max, def, zeros);
    }

    void add_bool_param(const char *path, bool def)
    {
      get_cur_tool()->addBool(path, def);
    }

    void add_description(const char *path)
    {
      get_cur_tool()->addDescription(path);
    }

    void add_string_list_param(const char *path, const char *value)
    {
      get_cur_tool()->addStringList(path, value);
    }

    void save_json()
    {
      std::ofstream(SCRIPT_FILE) << std::setw(4) << _json << "\n";
    }
  } // namespace scripting
} // namespace noggit