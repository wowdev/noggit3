#include <noggit/scripting/script_settings.hpp>
#include <noggit/scripting/script_profiles.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>

#include <sol/sol.hpp>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include <iomanip>

#define INNER_RADIUS_PATH "__inner_radius"
#define OUTER_RADIUS_PATH "__outer_radius"
#define SCRIPT_FILE "script_settings.json"

namespace noggit
{
  namespace scripting
  {
    script_settings::script_settings(scripting_tool *tool)
        : QGroupBox("Script Settings"), _tool(tool)
    {
      _layout = new QFormLayout(this);

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

      _layout->addWidget(_radius_group);

      connect(_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_slider);
        _radius_slider->setSliderPosition((int)std::round(v));
        set_json (OUTER_RADIUS_PATH, v);
      });

      connect(_radius_slider, &QSlider::valueChanged, [&](int v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_spin);
        _radius_spin->setValue(v);
        set_json (OUTER_RADIUS_PATH, v);
      });

      connect(_inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _inner_radius = v;
        QSignalBlocker const blocker(_inner_radius_slider);
        _inner_radius_slider->setSliderPosition((int)std::round(v * 100));
        set_json (INNER_RADIUS_PATH, v);
      });

      connect(_inner_radius_slider, &QSlider::valueChanged, [&](int v) {
        _inner_radius = v / 100.0f;
        QSignalBlocker const blocker(_inner_radius_spin);
        _inner_radius_spin->setValue(_inner_radius);
        set_json (INNER_RADIUS_PATH, _inner_radius);
      });
    }

    template <typename T>
    T script_settings::get_json_safe(std::string const& key, T def)
    {
      // TODO: this is terrible but accessing by reference didn't seem to work.
      auto ssn = _tool->get_context()->get_selected_name();
      auto prof = _tool->get_profiles()->get_cur_profile();

      if(!_json[ssn][prof].contains(key))
      {
        _json[ssn][prof][key] = def;
        return def;
      }
      return _json[ssn][prof][key];
    }

    template <typename T>
    T script_settings::get_json_unsafe(std::string const& key)
    {
      auto ssn = _tool->get_context()->get_selected_name();
      auto prof = _tool->get_profiles()->get_cur_profile();
      return (_json)[ssn][prof][key].get<T>();
    }

    template <typename T>
    void script_settings::set_json(std::string const& key, T def)
    {
      auto ssn = _tool->get_context()->get_selected_name();
      auto prof = _tool->get_profiles()->get_cur_profile();
      _json[ssn][prof][key] = def;
    }

// i don't remember why this was a macro
#define ADD_SLIDER(path, T, min, max, def, decimals)                                       \
  double dp1 = decimals > 0 ? decimals + 5 : decimals + 1;                                 \
  auto spinner = new QDoubleSpinBox(this);                                                 \
  spinner->setRange(min, max);                                                             \
  spinner->setDecimals(decimals);                                                          \
  spinner->setValue(def);                                                                  \
  auto slider = new QSlider(Qt::Orientation::Horizontal, this);                            \
  slider->setRange(min *dp1, max *dp1);                                                    \
  slider->setSliderPosition((int)std::round(def *dp1));                                    \
  auto label = new QLabel(this);                                                           \
  label->setText(name.c_str());                                                                    \
  connect(spinner, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double v) { \
    set_json<T>(path, (T)v);                                                        \
    QSignalBlocker const blocker(slider);                                                  \
    slider->setSliderPosition((int)std::round(v *dp1));                                    \
  });                                                                                      \
  connect(slider, &QSlider::valueChanged, [=](int v) {                               \
    double t = double(v) / dp1;                                                            \
    set_json<T>(path, t);                                                           \
    QSignalBlocker const blocker(spinner);                                                 \
    spinner->setValue(t);                                                                  \
  });                                                                                      \
                                                                                           \
  _layout->addRow(label, spinner);                                                         \
  _layout->addRow("", slider);                                                             \
  _widgets.push_back(label);                                                               \
  _widgets.push_back(spinner);                                                             \
  _widgets.push_back(slider);                                                              \
  set_json<T>(path, std::min(max, std::max(min, get_json_safe<T>(path, def))));       \
  auto v = get_json_safe<T>(path, def);                                                    \
  slider->setSliderPosition((int)std::round(v *dp1));                                      \
  spinner->setValue(v);

    double script_settings::get_double(std::string const& name)
    {
      return get_json_unsafe<double>(name);
    }

    int script_settings::get_int(std::string const& name)
    {
      return get_json_unsafe<int>(name);
    }

    bool script_settings::get_bool(std::string const& name)
    {
      return get_json_unsafe<bool>(name);
    }

    std::string script_settings::get_string(std::string const& name)
    {
      return get_json_unsafe<std::string>(name);
    }

    std::string script_settings::get_string_list(std::string const& name)
    {
      return get_json_unsafe<std::string>(name);
    }

    void script_settings::add_double(std::string const& name, double min, double max, double def, int zeros)
    {
      ADD_SLIDER(name, double, min, max, def, zeros);
    }

    void script_settings::add_int(std::string const& name, int min, int max, int def)
    {
      ADD_SLIDER(name, int, min, max, def, 0);
    }

    void script_settings::add_string(std::string const& name, std::string const& def)
    {
      auto tline = new QLineEdit(this);
      auto label = new QLabel(this);
      label->setText(name.c_str());
      connect(tline, &QLineEdit::textChanged, this, [=](auto text) {
        set_json<std::string>(name, text.toUtf8().constData());
      });
      _widgets.push_back(label);
      _widgets.push_back(tline);
      _layout->addRow(label, tline);
      tline->setText(get_json_safe<std::string>(name, def).c_str());
    }

    void script_settings::add_bool(std::string const& name, bool def)
    {
      auto checkbox = new QCheckBox(this);
      auto label = new QLabel(this);
      label->setText(name.c_str());
      connect(checkbox, &QCheckBox::stateChanged, this, [=](auto value) {
        set_json<bool>(name, value ? true : false);
      });

      _widgets.push_back(checkbox);
      _widgets.push_back(label);
      _layout->addRow(label, checkbox);

      checkbox->setCheckState(get_json_safe<bool>(name, def) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }



    void script_settings::add_string_list(std::string const& name, std::string const& value)
    {
      if (_string_arrays.find(name) == _string_arrays.end())
      {
        auto layout = new QFormLayout(this);
        auto box = new QComboBox(this);
        auto label = new QLabel(this);

        connect(box, QOverload<int>::of(&QComboBox::activated), this, [=](auto index) {
          set_json<std::string>(name, box->itemText(index).toUtf8().constData());
        });

        box->addItem(value.c_str());
        label->setText(name.c_str());

        _string_arrays[name] = box;
        _widgets.push_back(box);
        _widgets.push_back(label);
        _layout->addRow(label, box);

        // ensure there is at least one valid value in it
        get_json_safe<std::string>(name, value);
      }
      else
      {
        auto box = _string_arrays[name];
        box->addItem(value.c_str());

        // we found the last selection, so change the index for that.
        if (get_json_safe<std::string>(name, "") == value)
        {
          box->setCurrentIndex(box->count() - 1);
        }
      }
    }

    void script_settings::clear()
    {
      for (auto &widget : _widgets)
      {
        _layout->removeWidget(widget);
        delete widget;
      }
      _widgets.clear();
      _string_arrays.clear();
    }

    void script_settings::save_json()
    {
      std::ofstream(SCRIPT_FILE) << std::setw(4) << _json << "\n";
    }

    void script_settings::load_json()
    {
      if (!boost::filesystem::exists(SCRIPT_FILE))
      {
        return;
      }

      try
      {
        std::ifstream(SCRIPT_FILE) >> _json;
      }
      catch (std::exception err)
      {
        if (!boost::filesystem::exists(SCRIPT_FILE))
        {
          return;
        }
        // back up broken script settings, since they won't be read and will be overwritten.
        std::string backup_file = std::string(SCRIPT_FILE) + ".backup";
        int i = 0;
        while (boost::filesystem::exists(backup_file + std::to_string(i)))
        {
          ++i;
        }
        boost::filesystem::copy(SCRIPT_FILE, backup_file + std::to_string(i));
        // Add a message box here
      }
    }

    void script_settings::initialize()
    {
      double inner_radius = get_json_safe<double>(INNER_RADIUS_PATH, 0.5);
      double outer_radius = get_json_safe<double>(OUTER_RADIUS_PATH, 40);

      _radius_spin->setValue(outer_radius);
      _inner_radius_spin->setValue(inner_radius);

      _radius_slider->setValue(std::round(outer_radius));
      _inner_radius_slider->setSliderPosition((int)std::round(inner_radius * 100));

      for (auto& item : _string_arrays)
      {
        // if the index is 0, that MIGHT mean the value we have
        // is invalid (old script), so we just write it again to be safe.
        if (item.second->currentIndex() == 0)
        {
          set_json (item.first, item.second->itemText(0).toStdString());
        }
      }
    }

    nlohmann::json * script_settings::get_raw_json()
    {
      return &_json;
    }

    void register_settings(sol::state * state, scripting_tool * tool)
    {
      state->new_usertype<script_settings>("settings"
        , "add_double", &script_settings::add_double
        , "add_int", &script_settings::add_int
        , "add_bool", &script_settings::add_bool
        , "add_string", &script_settings::add_string
        , "add_string_list", &script_settings::add_string_list
        , "get_double", &script_settings::get_double
        , "get_int", &script_settings::get_int
        , "get_bool", &script_settings::get_bool
        , "get_string", &script_settings::get_string
        , "get_string_list", &script_settings::get_string_list
      );
    }
  } // namespace scripting
} // namespace noggit