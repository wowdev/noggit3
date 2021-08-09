// This file is part of Noggit3, licensed under GNU General Public License (version 3).
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
          script_settings * settings
        , math::vector_3d const& pos
        , float dt)
        : _settings(settings)
        , _pos(pos)
        , _dt(dt)
        {}

    math::vector_3d script_brush_event::pos()
    {
      return _pos;
    }

    float script_brush_event::outer_radius()
    {
      return _settings->brushRadius();
    }

    float script_brush_event::inner_radius()
    {
      return _settings->innerRadius();
    }

    void script_brush_event::set_outer_radius(float radius)
    {
      _settings->setOuterRadius(radius);
    }

    void script_brush_event::set_inner_radius(float radius)
    {
      _settings->setInnerRadius(radius);
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

    std::shared_ptr<int_tag> script_brush::add_int_tag(
        std::string const& item
      , int low
      , int high
      , int def
      , bool has_slider
      ){
      auto tag = std::make_shared<int_tag>(state(), _name, item, low, high, def, has_slider);
      _tags.push_back(tag);
      return tag;
    }

    void script_brush::add_null_tag()
    {
      add_description("");
    }

    void script_brush::add_description(std::string const& text)
    {
      _tags.push_back(std::make_shared<null_tag>(state(),_name,"__null"+std::to_string(_tags.size()),text));
    }

    std::shared_ptr<real_tag> script_brush::add_real_tag(
        std::string const& item
      , double low
      , double high
      , double def
      , int zeros
      , bool has_slider
      ){
      auto tag = std::make_shared<real_tag>(state(), _name, item, low, high, def, zeros, has_slider);
      _tags.push_back(tag);
      return tag;
    }

    std::shared_ptr<string_tag> script_brush::add_string_tag(
        std::string const& item
      , std::string const& def)
    {
      auto tag = std::make_shared<string_tag>(state(),_name, item, def);
      _tags.push_back(tag);
      return tag;
    }

    std::shared_ptr<string_list_tag> script_brush::add_string_list_tag(
        std::string const& item
      , sol::variadic_args va)
    {
      std::vector<std::string> vec;
      for(auto v : va)
      {
        vec.push_back(v);
      }
      auto tag = std::make_shared<string_list_tag>(state(),_name, item, vec);
      _tags.push_back(tag);
      return tag;
    }

    std::shared_ptr<bool_tag> script_brush::add_bool_tag(
        std::string const& item
      , bool def
    ) {
      auto tag = std::make_shared<bool_tag>(state(),_name,item,def);
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
        ,"set_outer_radius",&script_brush_event::set_outer_radius
        ,"set_inner_radius",&script_brush_event::set_inner_radius
        ,"outer_radius",&script_brush_event::outer_radius
        ,"inner_radius",&script_brush_event::inner_radius
        ,"dt",&script_brush_event::dt
        );

      state->new_usertype<script_brush>("script_brush"
        , sol::meta_function::new_index, &script_brush::set
        , sol::meta_function::index, &script_brush::get
        ,"get_name",&script_brush::get_name
        ,"add_int_tag",sol::overload(
            &script_brush::add_int_tag
          , &script_brush::add_int_tag_1
          , &script_brush::add_int_tag_2
          )
        ,"add_bool_tag",sol::overload(
            &script_brush::add_bool_tag
          , &script_brush::add_bool_tag_1
          )
        ,"add_real_tag", sol::overload(
            &script_brush::add_real_tag
          , &script_brush::add_real_tag_1
          , &script_brush::add_real_tag_2
        )
        ,"add_string_tag", sol::overload(
            &script_brush::add_string_tag
          , &script_brush::add_string_tag_1
        )
        ,"add_string_list_tag",&script_brush::add_string_list_tag
        ,"add_null_tag",&script_brush::add_null_tag
        ,"add_description",&script_brush::add_description
        );
    }
  }
}