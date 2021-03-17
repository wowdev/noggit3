#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_settings.hpp>

#include <string>
#include <map>

namespace noggit {
  namespace scripting {
      script_brush_event::script_brush_event(
        math::vector_3d const& pos
        , float outer_radius
        , float inner_radius
        , bool is_right
        , brush_event_type type 
        , float dt)
        : _pos(pos)
        , _outer_radius(outer_radius)
        , _inner_radius(inner_radius)
        , _is_right(is_right)
        , _type(type)
        , _dt(dt)
        {}

    math::vector_3d script_brush_event::pos()
    {
      return _pos;
    }

    float script_brush_event::outer_radius()
    {
      return _outer_radius;
    }

    float script_brush_event::inner_radius()
    {
      return _inner_radius;
    }

    bool script_brush_event::is_left()
    {
      return !_is_right;
    }

    bool script_brush_event::is_right()
    {
      return _is_right;
    }

    bool script_brush_event::is_click()
    {
      return _type==brush_event_type::CLICK;
    }

    bool script_brush_event::is_release()
    {
      return _type == brush_event_type::RELEASE;
    }

    bool script_brush_event::is_hold()
    {
      return _type == brush_event_type::HOLD;
    }

    float script_brush_event::dt()
    {
      return _dt;
    }

    script_brush::script_brush(
      scripting_tool * tool
      , std::string const& name
      , sol::protected_function select_event)
      : _tool(tool)
      , _name(name)
      , _select_event(select_event)
      {};

    void script_brush::set_name(std::string const& name)
    {
      _name = name;
    }

    void script_brush::on_click(sol::protected_function listener)
    {
      _click_event = listener;
    }

    std::string script_brush::get_name()
    {
      return _name;
    }

    void script_brush::send_click(script_brush_event const& evt)
    {
      if(_click_event)
      {
        _click_event(evt);
      }
    }

    script_settings * script_brush::settings()
    {
      return _tool->get_settings();
    }

    void register_script_brush(sol::state * state, scripting_tool * tool)
    {
    }
  }
}