#pragma once

#include <noggit/scripting/script_object.hpp>
#include <math/vector_3d.hpp>

#include <sol/sol.hpp>

#include <string>
#include <memory>

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_settings;
    class int_tag;
    class real_tag;
    class string_tag;
    class string_list_tag;
    class tag;
    class script_context;

    enum class brush_event_type
    {
      CLICK,
      HOLD,
      RELEASE
    };

    class script_brush_event
    {
    public:
      script_brush_event(math::vector_3d const& pos
                        , float outer_radius
                        , float inner_radius
                        , float dt);
      math::vector_3d pos();
      float outer_radius();
      float inner_radius();
      float dt();
    private:
      math::vector_3d _pos;
      float _outer_radius;
      float _inner_radius;
      float _dt;
    };

    class script_brush: public script_object {
    public:
      script_brush(script_context * state, std::string const& name);
      void set_name(std::string const& name);
      std::string get_name();
      void on_selected();

      std::shared_ptr<int_tag> add_int_tag(
        std::string const& item
        , int low
        , int high
        , int def
        , bool has_slider = false
        );

      std::shared_ptr<real_tag> add_real_tag(std::string const& item
                                            , double low
                                            , double high
                                            , double def
                                            , int zeros = 5
                                            , bool has_slider = false
                                            );

      std::shared_ptr<string_tag> add_string_tag(
          std::string const& item
        , std::string const& def);
                                                
      std::shared_ptr<string_list_tag> add_string_list_tag(
          std::string const& item
        , sol::variadic_args va);

      void add_null_tag();

      LUA_MEMBER_FUNC(
        script_brush,std::shared_ptr<script_brush_event>,on_left_click);
      LUA_MEMBER_FUNC(
        script_brush,std::shared_ptr<script_brush_event>,on_left_hold);
      LUA_MEMBER_FUNC(
        script_brush,std::shared_ptr<script_brush_event>,on_left_release);
      LUA_MEMBER_FUNC(
        script_brush,std::shared_ptr<script_brush_event>,on_right_click);
      LUA_MEMBER_FUNC(
        script_brush,std::shared_ptr<script_brush_event>,on_right_hold);
      LUA_MEMBER_FUNC(
        script_brush,std::shared_ptr<script_brush_event>,on_right_release);

    private:
      std::vector<std::shared_ptr<tag>> _tags;
      std::string _name;
    };

    void register_script_brush(script_context * state);
  }
}