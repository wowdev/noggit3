#pragma once

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

      void send_left_click(script_brush_event evt);
      void send_left_hold(script_brush_event evt);
      void send_left_release(script_brush_event evt);

      void send_right_click(script_brush_event evt);
      void send_right_hold(script_brush_event evt);
      void send_right_release(script_brush_event evt);

      // <script_brush_event>
      sol::protected_function _left_click = nullptr;
      sol::protected_function _left_hold = nullptr;
      sol::protected_function _left_release = nullptr;

      sol::protected_function _right_click = nullptr;
      sol::protected_function _right_hold = nullptr;
      sol::protected_function _right_release = nullptr;

    private:
      // <script_brush>
      sol::protected_function _select_event;

      std::string _name;
      scripting_tool * _tool;
    };

    void register_script_brush(sol::state * state, scripting_tool * tool);
  }
}