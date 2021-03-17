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
        , bool is_right
        , brush_event_type type
        , float dt);

        math::vector_3d pos();
        float outer_radius();
        float inner_radius();
        bool is_left();
        bool is_right();
        bool is_click();
        bool is_hold();
        bool is_release();
        float dt();
    private:
      math::vector_3d _pos;
      float _outer_radius;
      float _inner_radius;
      bool _is_right;
      bool _is_click;
      float _dt;
      brush_event_type _type;
    };

    class script_brush {
    public:
      script_brush(scripting_tool * tool, std::string const& name, sol::protected_function select_event);
      void set_name(std::string const& name);
      void on_click(sol::protected_function listener);
      void send_click(script_brush_event const& event);
      std::string get_name();
      script_settings * settings();

    private:
      // <scripting_tool>
      sol::protected_function _select_event;
      // <click_context>
      sol::protected_function _click_event = nullptr;
      std::string _name;
      scripting_tool * _tool;
    };

    void register_script_brush(sol::state * state, scripting_tool * tool);
  }
}