#pragma once

#include <nlohmann/json.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>

namespace sol {
  class state;
}

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_profiles;

    class script_settings : public QGroupBox {
    public:
      script_settings(scripting_tool* tool);
      void add_double(char const* name, double min, double max, double def = 0, int zeros = 2);
      void add_int(char const* name, int min, int max, int def = 0);
      void add_bool(char const* name, bool def = false);
      void add_string(char const* name, char const* def = "");
      void add_string_list(char const* name, char const* value);

      double get_double(char const* name);
      int get_int(char const* name);
      bool get_bool(char const* name);
      std::string get_string(char const* name);
      std::string get_string_list(char const* name);

      float brushRadius() const { return _radius; }
      float innerRadius() const { return _inner_radius; }

      void clear();

      void save_json();
      void load_json();

      nlohmann::json * get_raw_json();

      void initialize();
    private:
      template <typename T>
      T get_json_safe(std::string key, T def);
      template <typename T>
      T get_json_unsafe(std::string key);
      template <typename T>
      void set_json(std::string key, T def);

      QGroupBox* _radius_group;
      QFormLayout* _radius_layout;
      QDoubleSpinBox* _radius_spin;
      QSlider* _radius_slider;

      QDoubleSpinBox* _inner_radius_spin;
      QSlider* _inner_radius_slider;

      QFormLayout* _layout;

      float _radius = 0;
      float _inner_radius = 0;
    private:
      noggit::scripting::scripting_tool* _tool;
      std::vector<QWidget*> _widgets;
      nlohmann::json _json;
      std::map<std::string, QComboBox*> _string_arrays;
    };

    void register_settings(sol::state * state, scripting_tool * tool);
  }
}