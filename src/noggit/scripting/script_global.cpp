#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_global.hpp>
#include <noggit/MapView.h>
#include <noggit/World.h>

#include <noggit/scripting/script_context.hpp>

namespace noggit {
  namespace scripting {
    void register_global(sol::state * state, scripting_tool * global)
    {
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
        global->get_view()->_world.get()->addM2(filename,pos,scale,math::degrees::vec3(rotation), nullptr);
      });

      state->set_function("add_wmo",[global](
        scripting_tool * global
        , std::string const& filename
        , math::vector_3d const& pos
        , math::vector_3d const& rotation)
      {
        global->get_view()->_world.get()->addWMO(filename,pos,math::degrees::vec3(rotation));
      });

      state->set_function("get_map_id",[global]()
      {
        return global->get_view()->_world.get()->getMapID();
      });

      state->set_function("get_area_id",[global](math::vector_3d const& pos)
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

      state->set_function("print",[global](std::string const& msg)
      {
        return global->addLog(msg);
      });
    }
  }
}