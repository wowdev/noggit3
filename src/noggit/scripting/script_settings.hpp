// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>

#include <nlohmann/json.hpp>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_profiles;
    class script_context;

    class script_settings : public QGroupBox {
    public:
      script_settings(scripting_tool* tool);
      void add_double( std::string const& name
                     , double min
                     , double max
                     , double def = 0
                     , int zeros = 2
                     , bool has_slider = false
                     );

      void add_int( std::string const& name
                  , int min
                  , int max
                  , int def = 0
                  , bool has_slider = false
                  );

      void add_bool( std::string const& name, bool def = false);
      void add_null( std::string const& text = "");
      void add_string(std::string const& name, std::string const& def = "");
      void add_string_list(std::string const& name, std::string const& value);
      void add_description(std::string const& description);

      double get_double(std::string const& name);
      int get_int(std::string const& name);
      bool get_bool(std::string const& name);
      std::string get_string(std::string const& name);
      std::string get_string_list(std::string const& name);

      float brushRadius() const { return _radius; }
      float innerRadius() const { return _inner_radius; }

      void setOuterRadius(float outerRadius);
      void setInnerRadius(float innerRadius);

      void clear();

      void save_json();
      void load_json();

      nlohmann::json * get_raw_json();

      template <typename T>
      T get_setting(std::string const& script
                   , std::string const& profile
                   , std::string const& item
                   , T def
                   )
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
      T get_json_safe(std::string const& key, T def, std::function<bool(nlohmann::basic_json<>)>);
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

      QFrame* _custom_group;
      QFormLayout* _custom_layout;

      float _radius = 0;
      float _inner_radius = 0;
    private:
      noggit::scripting::scripting_tool* _tool;
      std::vector<QWidget*> _widgets;
      nlohmann::json _json;
      std::map<std::string, QComboBox*> _string_arrays;
    };

    class tag : public script_object {
      public:
        tag( script_context * ctx
           , std::string const& script
           , std::string const& item
           );
        virtual void add_to_settings() = 0;
      protected:
        scripting_tool * _tool;
        std::string _script;
        std::string _item;
      private:
    };

    class int_tag : public tag {
      public:
        int_tag(script_context * ctx
               , std::string const& script
               , std::string const& item
               , int min
               , int max
               , int def
               , bool has_slider
               );
        int get();
        void set(int value);
        virtual void add_to_settings() override;

      private:
        int _min,_max,_def;
        bool _has_slider;
    };

    class real_tag : public tag {
      public:
        real_tag( script_context * ctx
                  , std::string const& script
                  , std::string const& item
                  , double min
                  , double max
                  , double def
                  , int zeros
                  , bool has_slider
                  );
        double get();
        virtual void add_to_settings() override;
      private:
        double _min,_max,_def;
        int _zeros;
        bool _has_slider;
    };

    class null_tag : public tag {
    public:
      null_tag(script_context* ctx
        , std::string const& script
        , std::string const& item
        , std::string const& text = ""
      );
      virtual void add_to_settings() override;
    private:
      std::string _text;
    };

    class string_tag : public tag {
      public:
        string_tag( script_context * ctx
                  , std::string const& script
                  , std::string const& item
                  , std::string const& def
                  );
        std::string get();
        virtual void add_to_settings() override;
      private:
        std::string _def;
    };

    class string_list_tag : public tag {
      public:
        string_list_tag( script_context * ctx
                       , std::string const& script
                       , std::string const& item
                       , std::vector<std::string> const& strings
                       );
        std::string get();
        virtual void add_to_settings() override;
      private:
        std::vector<std::string> _values;
    };

    class bool_tag : public tag {
      public:
        bool_tag( script_context * ctx
                , std::string const& script
                , std::string const& item
                , bool def
                );
        virtual void add_to_settings() override;
        bool get();
      private:
        bool _def;
    };

    void register_settings(script_context * state);
  }
}