// This file is part of Noggit3, licensed under GNU General Public License (version 3).
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
    float cam_pitch (das::Context* context)
    {
      // TODO: Fix
      //return get_ctx(context, "cam_pitch")->_camera->pitch()._;
      return 0.0;
    }

    float outer_radius (das::Context* context)
    {
      // TODO: Fix
      return 0.0;
      //return get_ctx(context, "outer_radius")->_outer_radius;
    }

    float inner_radius (das::Context* context)
    {
      // TODO: Fix
      //return get_ctx(context, "inner_radius")->_inner_radius;
      return 0.0;
    }

    float cam_yaw (das::Context* context)
    {
      // TODO: Fix
      return 0.0;
      //return get_ctx(context, "cam_yaw")->_camera->yaw()._;
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
      // TODO: Fix
      //get_ctx(context, "add_m2")->_world->addM2(filename, pos, scale, math::degrees::vec3 {rotation}, &p);
    }

    void add_wmo(char const* filename, math::vector_3d const& pos, math::vector_3d const& rotation)
    {
      if(filename==nullptr)
      {
        throw script_exception(
          "add_wmo",
          "empty string parameter (in call to add_wmo)");
      }
      // TODO: Fix
      //get_ctx(context, "add_wmo")->_world->addWMO(filename, pos, math::degrees::vec3 {rotation});
    }

    float dt ()
    {
      // TODO: Fix
      return 0.0;
      //return get_ctx(context, "dt")->_dt;
    }

    unsigned int get_map_id ()
    {
      // TODO: Fix
      return 0;
      //return get_ctx(context, "get_map_id")->_world->getMapID();
    }

    unsigned int get_area_id(math::vector_3d const& pos)
    {
      // TODO: Fix
      return 0;
      //return get_ctx (context, "get_area_id")->_world->getAreaID(pos);
    }
    bool holding_alt ()
    {
      // TODO: Fix
      return 0;
      //return get_ctx (context, "holding_alt")->_holding_alt;
    }
    bool holding_shift ()
    {
      // TODO: Fix
      return false;
      //return get_ctx (context, "holding_shift")->_holding_shift;
    }
    bool holding_ctrl ()
    {
      // TODO: Fix
      return false;
      //return get_ctx (context, "holding_ctrl")->_holding_ctrl;
    }
    bool holding_space ()
    {
      // TODO: Fix
      return false;
      //return get_ctx (context, "holding_space")->_holding_space;
    }
    math::vector_3d pos ()
    {
      // TODO: Fix
      return math::vector_3d(0, 0, 0);
      //return get_ctx (context, "pos")->_pos;
    }
  } // namespace scripting
} // namespace noggit
