// This file is part of Noggit3, licensed under GNU General Public License (version 3).
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

      _custom_group = new QFrame();
      _custom_layout = new QFormLayout(_custom_group);

      _layout->addWidget(_radius_group);
      _layout->addWidget(_custom_group);

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
    T script_settings::get_json_safe(std::string const& key, T def, std::function<bool(nlohmann::basic_json<>)> check)
    {
      // TODO: this is terrible but accessing by reference didn't seem to work.
      auto ssn = _tool->get_context()->get_selected_name();
      auto prof = _tool->get_profiles()->get_cur_profile();

      if(!_json[ssn][prof].contains(key))
      {
        _json[ssn][prof][key] = def;
        return def;
      }

      auto v = _json[ssn][prof][key];
      if (!check(v))
      {
        _json[ssn][prof][key] = def;
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
#define ADD_SLIDER(path, T, min, max, def, decimals, has_slider)                           \
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
  _custom_layout->addRow(label, spinner);                                                         \
  if(has_slider) _custom_layout->addRow("", slider);                                              \
  else slider->hide();                                                                     \
  _widgets.push_back(label);                                                               \
  _widgets.push_back(spinner);                                                             \
  _widgets.push_back(slider);                                                              \
  set_json<T>(path, std::min(max, std::max(min, get_json_safe<T>(path, def, [](auto json){ return json.is_number();}))));       \
  auto v = get_json_safe<T>(path, def,[](auto json){ return json.is_number(); });                                                    \
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

    void script_settings::add_double(
        std::string const& name
      , double min
      , double max
      , double def
      , int zeros
      , bool has_slider
      ){
        ADD_SLIDER(name, double, min, max, def, zeros, has_slider);
      }

    void script_settings::add_int(
        std::string const& name
      , int min
      , int max
      , int def
      , bool has_slider
      ){
        ADD_SLIDER(name, int, min, max, def, 0, has_slider);
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
      tline->setText(get_json_safe<std::string>(name, def, [](auto json) {return json.is_string(); }).c_str());
      _custom_layout->addRow(label, tline);
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
      _custom_layout->addRow(label, checkbox);

      checkbox->setCheckState(get_json_safe<bool>(name, def, [](auto json) { return json.is_boolean(); }) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }

    void script_settings::add_null(std::string const& text)
    {
      auto label = new QLabel(this);
      label->setText(text.c_str());
      _widgets.push_back(label);
      _custom_layout->addRow(label);
    }

    void script_settings::add_string_list(std::string const& name, std::string const& value)
    {
      if (_string_arrays.find(name) == _string_arrays.end())
      {
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
        _custom_layout->addRow(label, box);

        // ensure there is at least one valid value in it
        get_json_safe<std::string>(name, value, [](auto json) {return json.is_string(); });
      }
      else
      {
        auto box = _string_arrays[name];
        box->addItem(value.c_str());

        // we found the last selection, so change the index for that.
        if (get_json_safe<std::string>(name, "", [](auto json) {return json.is_string(); }) == value)
        {
          box->setCurrentIndex(box->count() - 1);
        }
      }
    }

    void script_settings::setOuterRadius(float outerRadius)
    {
      _radius_spin->setValue(outerRadius);
      _radius_slider->setSliderPosition(outerRadius);
      set_json (OUTER_RADIUS_PATH, outerRadius);
    }

    void script_settings::setInnerRadius(float innerRadius)
    {
      _inner_radius_spin->setValue(innerRadius);
      _inner_radius_slider->setSliderPosition(innerRadius*100);
      set_json (INNER_RADIUS_PATH, innerRadius);
    }

    void script_settings::clear()
    {
      for (auto &widget : _widgets)
      {
        _custom_layout->removeWidget(widget);
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
      double inner_radius = get_json_safe<double>(INNER_RADIUS_PATH, 0.5, [](auto json) {return json.is_number(); });
      double outer_radius = get_json_safe<double>(OUTER_RADIUS_PATH, 40, [](auto json) {return json.is_number(); });

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

    tag::tag( script_context * ctx
            , std::string const& script
            , std::string const& item
            )
            : script_object(ctx)
            , _tool(ctx->tool())
            , _script(script)
            , _item(item)
      {}

    int_tag::int_tag(script_context * ctx
                    , std::string const& script
                    , std::string const& item
                    , int min
                    , int max
                    , int def
                    , bool has_slider
                    )
                    : tag(ctx,script,item)
                    , _min(min)
                    , _max(max)
                    , _def(def)
                    , _has_slider(has_slider)
    {}

    int int_tag::get()
    {
      return _tool->get_settings()->get_setting<int>(
        _script,_tool->get_profiles()->get_cur_profile(),_item,_def);
    }
    
    void int_tag::add_to_settings()
    {
      _tool->get_settings()->add_int(_item, _min,_max,_def);
    }

    null_tag::null_tag(
        script_context * ctx
      , std::string const& script
      , std::string const& item
      , std::string const& text
      )
      : tag(ctx,script,item)
      , _text(text)
      {}

    void null_tag::add_to_settings()
    {
      _tool->get_settings()->add_null(_text);
    }

    real_tag::real_tag(script_context * ctx
                      , std::string const& script
                      , std::string const& item
                      , double min
                      , double max
                      , double def
                      , int zeros
                      , bool has_slider
                      )
                      : tag(ctx,script,item)
                      , _min(min)
                      , _max(max)
                      , _def(def)
                      , _zeros(zeros)
                      , _has_slider(has_slider)
    {}


    double real_tag::get()
    {
      return _tool->get_settings()->get_setting<double>(
        _script,_tool->get_profiles()->get_cur_profile(),_item,_def);
    }

    void real_tag::add_to_settings()
    {
      _tool->get_settings()->add_double(_item, _min,_max,_def, _zeros, _has_slider);
    }

    string_tag::string_tag(script_context * ctx
                          , std::string const& script
                          , std::string const& item
                          , std::string const& def
                          )
                          : tag(ctx,script,item)
                          , _def(def)
    {}

    std::string string_tag::get()
    {
      return _tool->get_settings()->get_setting<std::string>(
        _script,_tool->get_profiles()->get_cur_profile(),_item,_def);
    }

    void string_tag::add_to_settings()
    {
      _tool->get_settings()->add_string(_item ,_def);
    }

    string_list_tag::string_list_tag(script_context * ctx
                                    , std::string const& script
                                    , std::string const& item
                                    , std::vector<std::string> const& values
                                    )
      : tag(ctx,script,item)
      , _values(values)
    {}

    std::string string_list_tag::get()
    {
      return _tool->get_settings()->get_setting<std::string>(
        _script,_tool->get_profiles()->get_cur_profile(),_item,_values[0]);
    }

    void string_list_tag::add_to_settings()
    {
      for(auto& val: _values)
      {
        _tool->get_settings()->add_string_list(_item, val);
      }
    }

    bool_tag::bool_tag(
        script_context * ctx
      , std::string const& script
      , std::string const& item
      , bool def
      )
      : tag(ctx,script,item)
      , _def(def)
    {}

    bool bool_tag::get()
    {
      return _tool->get_settings()->get_setting<bool>
      (
        _script,_tool->get_profiles()->get_cur_profile(),_item,_def
      );
    }

    void bool_tag::add_to_settings()
    {
      _tool->get_settings()->add_bool(_item,_def);
    }

    void register_settings(script_context * state)
    {
      state->new_usertype<int_tag>("int_tag","get",&int_tag::get);
      state->new_usertype<bool_tag>("bool_tag", "get", &bool_tag::get);
      state->new_usertype<real_tag>("real_tag","get",&real_tag::get);
      state->new_usertype<string_tag>("string_tag","get",&string_tag::get);
      state->new_usertype<string_list_tag>("string_list_tag","get",&string_list_tag::get);
    }
  } // namespace scripting
} // namespace noggit