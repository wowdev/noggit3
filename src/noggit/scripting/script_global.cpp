// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_global.hpp>
#include <noggit/MapView.h>
#include <noggit/World.h>

#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_chunk.hpp>

#include <noggit/ui/ObjectEditor.h>

namespace noggit {
  namespace scripting {
    void register_global(script_context * state)
    {
      scripting_tool * global = state->tool();
      state->set_function("camera_pos",[global]()
      { 
        return global->get_view()->_camera.position;
      });

      state->set_function("add_m2",[global](
        std::string const& filename
        , math::vector_3d const& pos
        , float scale
        , math::vector_3d const& rotation)
      { 
        // note: we set both min/max random scale and the normal scale parameter,
        // because noggit picks one based on random scale settings in the object tool
        object_paste_params p;
        p.minScale = scale;
        p.maxScale = scale;
        global->get_view()->_world.get()->
          addM2(filename,pos,scale,math::degrees::vec3(rotation), &p);
      });

      state->set_function("vec",[](float x, float y, float z){
        return math::vector_3d(x,y,z);
      });

      state->set_function("add_wmo",[global](
          std::string const& filename
        , math::vector_3d const& pos
        , math::vector_3d const& rotation)
      {
        global->get_view()->_world.get()->addWMO(
            filename
          , pos
          , math::degrees::vec3(rotation));
      });

      state->set_function("get_map_id",[global]()
      {
        return global->get_view()->_world.get()->getMapID();
      });

      state->set_function("get_area_id",[global](
        math::vector_3d const& pos)
      {
        return global->get_view()->_world.get()->getAreaID(pos);
      });

      state->set_function("cam_pitch",[global]()
      {
        return global->get_view()->_camera.pitch()._;
      });

      state->set_function("cam_yaw",[global]()
      {
        return global->get_view()->_camera.yaw()._;
      });

      state->set_function("holding_alt",[global]()
      {
        return global->get_view()->_mod_alt_down;
      });

      state->set_function("holding_shift",[global]()
      {
        return global->get_view()->_mod_shift_down;
      });

      state->set_function("holding_ctrl",[global]()
      {
        return global->get_view()->_mod_ctrl_down;
      });

      state->set_function("holding_space",[global]()
      {
        return global->get_view()->_mod_space_down;
      });

      state->set_function("holding_left_mouse",[global]()
      {
        return global->get_view()->leftMouse;
      });

      state->set_function("holding_right_mouse",[global]()
      {
        return global->get_view()->rightMouse;
      });

      state->set_function("print",[global](sol::variadic_args va)
      {
        std::string str = "";
        for(auto v : va)
        {
          switch (v.get_type())
          {
          case sol::type::boolean:
            str += std::to_string((bool)v);
            break;
          case sol::type::number:
            str += std::to_string(v.get<float>());
            break;
          case sol::type::string:
            str += v.get<std::string>();
            break;
          case sol::type::table:
            // TODO: implement
            str += "{table}";
            break;
          }
        }
        return global->addLog(str);
      });
    }
  }
}