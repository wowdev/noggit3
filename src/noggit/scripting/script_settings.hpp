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
    class lua_state;

    class script_settings : public QGroupBox {
    public:
      script_settings(scripting_tool* tool);
      void add_double(std::string const& name, double min, double max, double def = 0, int zeros = 2);
      void add_int(std::string const& name, int min, int max, int def = 0);
      void add_bool(std::string const& name, bool def = false);
      void add_string(std::string const& name, std::string const& def = "");
      void add_string_list(std::string const& name, std::string const& value);

      double get_double(std::string const& name);
      int get_int(std::string const& name);
      bool get_bool(std::string const& name);
      std::string get_string(std::string const& name);
      std::string get_string_list(std::string const& name);

      float brushRadius() const { return _radius; }
      float innerRadius() const { return _inner_radius; }

      void clear();

      void save_json();
      void load_json();

      nlohmann::json * get_raw_json();

      template <typename T>
      T get_setting(std::string const& script, std::string const& profile, std::string const& item, T def)
      {
        if(!_json[script][profile].contains(item))
        {
          _json[script][profile][item] = def;
          return def;
        }
        return _json[script][profile][item];
      }

      void initialize();
    private:
      template <typename T>
      T get_json_safe(std::string const& key, T def);
      template <typename T>
      T get_json_unsafe(std::string const& key);
      template <typename T>
      void set_json(std::string const& key, T def);

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

    class tag {
      public:
        tag(std::string const& script, std::string const& item, scripting_tool * tool)
          : _script(script)
          , _item(item)
          , _tool(tool)
          {}
          virtual void add_to_settings() = 0;

      protected:
        scripting_tool * _tool;
        std::string _script;
        std::string _item;
      private:
    };

    class int_tag : public tag {
      public:
        int_tag(std::string const& script, std::string const& item, scripting_tool * tool, int min, int max, int def);
        int get();
        virtual void add_to_settings() override;

      private:
        int _min,_max,_def;
    };

    class real_tag : public tag {
      public:
        real_tag(std::string const& script, std::string const& item, scripting_tool * tool, double min, double max, double def);
        double get();
        virtual void add_to_settings() override;
      private:
        double _min,_max,_def;
    };

    class string_tag : public tag {
      public:
        string_tag(std::string const& script, std::string const& item, scripting_tool * tool, std::string const& def);
        std::string get();
        virtual void add_to_settings() override;
      private:
        std::string _def;
    };

    class string_list_tag : public tag {
      public:
        string_list_tag(std::string const& script, std::string const& item, scripting_tool * tool, std::vector<std::string> const& strings);
        std::string get();
        virtual void add_to_settings() override;
      private:
        std::vector<std::string> _values;
    };

    void register_settings(lua_state * state);
  }
}