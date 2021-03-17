#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_global.hpp>
#include <noggit/MapView.h>
#include <noggit/World.h>

#include <noggit/scripting/script_context.hpp>

namespace noggit {
  namespace scripting {
    math::vector_3d camera_pos(scripting_tool * global)
    {
      return global->get_view()->_camera.position;
    }

    void add_m2(
      scripting_tool * global
      , char const* filename
      , math::vector_3d const& pos
      , float scale
      , math::vector_3d const& rotation)
    {
      global->get_view()->_world.get()->addM2(filename,pos,scale,math::degrees::vec3(rotation), nullptr);
    }

    void add_wmo(
      scripting_tool * global
      , char const* filename
      , math::vector_3d const& pos
      , math::vector_3d const& rotation)
    {
      global->get_view()->_world.get()->addWMO(filename,pos,math::degrees::vec3(rotation));
    }

    unsigned int get_map_id(scripting_tool * global)
    {
      return global->get_view()->_world.get()->getMapID();
    }

    unsigned int get_area_id(scripting_tool * global, math::vector_3d const& pos)
    {
      return global->get_view()->_world.get()->getAreaID(pos);
    }

    float cam_pitch(scripting_tool * global)
    {
      return global->get_view()->_camera.pitch()._;
    }

    float cam_yaw(scripting_tool * global)
    {
      return global->get_view()->_camera.yaw()._;
    }

    bool holding_alt(scripting_tool * global)
    {
      return global->get_view()->_mod_alt_down;
    }

    bool holding_shift(scripting_tool * global)
    {
      return global->get_view()->_mod_shift_down;
    }

    bool holding_ctrl(scripting_tool * global)
    {
      return global->get_view()->_mod_ctrl_down;
    }

    bool holding_space(scripting_tool * global)
    {
      return global->get_view()->_mod_space_down;
    }

    bool holding_left_mouse(scripting_tool * global)
    {
      return global->get_view()->leftMouse;
    }

    bool holding_right_mouse(scripting_tool * global)
    {
      return global->get_view()->rightMouse;
    }
  }
}