// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <daScript/daScript.h> // must be on top

#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/camera.hpp>
#include <noggit/scripting/script_misc.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <lodepng.h>

namespace noggit
{
  namespace scripting
  {
    float cam_pitch()
    {
      return get_ctx("cam_pitch")->_camera->pitch()._;
    }

    float outer_radius()
    {
      return get_ctx("outer_radius")->_outer_radius;
    }

    float inner_radius()
    {
      return get_ctx("inner_radius")->_inner_radius;
    }

    float cam_yaw()
    {
      return get_ctx("cam_yaw")->_camera->yaw()._;
    }

    math::vector_3d vec(float x, float y, float z)
    {
      return math::vector_3d(x, y, z);
    }

    void add_m2(char const* filename, math::vector_3d const& pos, float scale, math::vector_3d const& rotation)
    {
      if(filename==nullptr)
      {
        throw script_exception(
          "add_m2",
          "empty string parameter (in call to add_m2)"
          );
      }
      auto p = object_paste_params();
      get_ctx("add_m2")->_world->addM2(filename, pos, scale, rotation,& p);
    }

    void add_wmo(char const* filename, math::vector_3d const& pos, math::vector_3d const& rotation)
    {
      if(filename==nullptr)
      {
        throw script_exception(
          "add_wmo",
          "empty string parameter (in call to add_wmo)");
      }
      get_ctx("add_wmo")->_world->addWMO(filename, pos, rotation);
    }

    float dt()
    {
      return get_ctx("dt")->_dt;
    }

    unsigned int get_map_id()
    {
      return get_ctx("get_map_id")->_world->getMapID();
    }

    unsigned int get_area_id(math::vector_3d const& pos)
    {
      return get_ctx("get_area_id")->_world->getAreaID(pos);
    }
    bool holding_alt()
    {
      return get_ctx("holding_alt")->_holding_alt;
    }
    bool holding_shift()
    {
      return get_ctx("holding_shift")->_holding_shift;
    }
    bool holding_ctrl()
    {
      return get_ctx("holding_ctrl")->_holding_ctrl;
    }
    bool holding_space()
    {
      return get_ctx("holding_space")->_holding_space;
    }
    math::vector_3d pos()
    {
      return get_ctx("pos")->_pos;
    }
  } // namespace scripting
} // namespace noggit
