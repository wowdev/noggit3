#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_settings.hpp>
#include <noggit/scripting/script_profiles.hpp>
#include <noggit/scripting/script_context.hpp>

#include <string>
#include <map>

namespace noggit {
  namespace scripting {
      script_brush_event::script_brush_event(
        math::vector_3d const& pos
        , float outer_radius
        , float inner_radius
        , float dt)
        : _pos(pos)
        , _outer_radius(outer_radius)
        , _inner_radius(inner_radius)
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

    float script_brush_event::dt()
    {
      return _dt;
    }

    script_brush::script_brush(
      script_context * state
      , std::string const& name)
      : script_object(state)
      , _name(name)
      {};

    void script_brush::set_name(std::string const& name)
    {
      _name = name;
    }

    std::string script_brush::get_name()
    {
      return _name;
    }

    std::shared_ptr<int_tag> script_brush::add_int_tag(std::string const& item, int low, int high, int def)
    {
      auto tag = std::make_shared<int_tag>(_name, item, state()->tool(), low, high, def);
      _tags.push_back(tag);
      return tag;
    }

    std::shared_ptr<real_tag> script_brush::add_real_tag(std::string const& item, double low, double high, double def)
    {
      auto tag = std::make_shared<real_tag>(_name, item, state()->tool(), low, high, def);
      _tags.push_back(tag);
      return tag;
    }

    std::shared_ptr<string_tag> script_brush::add_string_tag(std::string const& item, std::string const& def)
    {
      auto tag = std::make_shared<string_tag>(_name, item, state()->tool(), def);
      _tags.push_back(tag);
      return tag;
    }

    std::shared_ptr<string_list_tag> script_brush::add_string_list_tag(std::string const& item, sol::variadic_args va)
    {
      std::vector<std::string> vec;
      for(auto v : va)
      {
        vec.push_back(v);
      }
      auto tag = std::make_shared<string_list_tag>(_name, item, state()->tool(), vec);
      _tags.push_back(tag);
      return tag;
    }

    void script_brush::on_selected()
    {
      for(auto tag : _tags)
      {
        tag->add_to_settings();
      }
    }

    void register_script_brush(script_context * state)
    {
      state->new_usertype<script_brush_event>("script_brush_event"
        ,"pos",&script_brush_event::pos
        ,"outer_radius",&script_brush_event::outer_radius
        ,"inner_radius",&script_brush_event::inner_radius
        ,"dt",&script_brush_event::dt
        );

      state->new_usertype<script_brush>("script_brush"
        , sol::meta_function::new_index, &script_brush::set
        , sol::meta_function::index, &script_brush::get
        ,"get_name",&script_brush::get_name
        ,"add_int_tag",&script_brush::add_int_tag
        ,"add_real_tag",&script_brush::add_real_tag
        ,"add_string_tag",&script_brush::add_string_tag
        ,"add_string_list_tag",&script_brush::add_string_list_tag
        );
    }
  }
}