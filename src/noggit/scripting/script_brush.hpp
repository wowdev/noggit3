#pragma once

#include <noggit/scripting/lua_function.hpp>

#include <sol/sol.hpp>
#include <string>
#include <math/vector_3d.hpp>

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_settings;
    enum class brush_event_type
    {
      CLICK,
      HOLD,
      RELEASE
    };

    class script_brush_event
    {
    public:
      script_brush_event(
        math::vector_3d const& pos
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

    class script_brush {
    public:
      script_brush(scripting_tool * tool, std::string const& name, sol::protected_function select_event);
      void set_name(std::string const& name);
      std::string get_name();
      script_settings * settings();

      void on_left_click(sol::protected_function fn);
      void on_left_hold(sol::protected_function fn);
      void on_left_release(sol::protected_function fn);
      void on_right_click(sol::protected_function fn);
      void on_right_hold(sol::protected_function fn);
      void on_right_release(sol::protected_function fn);

      lua_function<script_brush*> _select;
      lua_function<script_brush_event const&> _left_click;
      lua_function<script_brush_event const&> _left_hold;
      lua_function<script_brush_event const&> _left_release;
      lua_function<script_brush_event const&> _right_click;
      lua_function<script_brush_event const&> _right_hold;
      lua_function<script_brush_event const&> _right_release;
    private:
      std::string _name;
      scripting_tool * _tool;
    };

    void register_script_brush(sol::state * state, scripting_tool * tool);
  }
}