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
      return get_ctx(context, "cam_pitch")->_camera->pitch()._;
    }

    float outer_radius (das::Context* context)
    {
      return get_ctx(context, "outer_radius")->_outer_radius;
    }

    float inner_radius (das::Context* context)
    {
      return get_ctx(context, "inner_radius")->_inner_radius;
    }

    float cam_yaw (das::Context* context)
    {
      return get_ctx(context, "cam_yaw")->_camera->yaw()._;
    }

    math::vector_3d vec(float x, float y, float z)
    {
      return math::vector_3d(x, y, z);
    }

    void add_m2(char const* filename, math::vector_3d const& pos, float scale, math::vector_3d const& rotation, das::Context* context)
    {
      if(filename==nullptr)
      {
        throw script_exception(
          "add_m2",
          "empty string parameter (in call to add_m2)"
          );
      }
      auto p = object_paste_params();
      get_ctx(context, "add_m2")->_world->addM2(filename, pos, scale, math::degrees::vec3 {rotation}, &p);
    }

    void add_wmo(char const* filename, math::vector_3d const& pos, math::vector_3d const& rotation, das::Context* context)
    {
      if(filename==nullptr)
      {
        throw script_exception(
          "add_wmo",
          "empty string parameter (in call to add_wmo)");
      }
      get_ctx(context, "add_wmo")->_world->addWMO(filename, pos, math::degrees::vec3 {rotation});
    }

    float dt (das::Context* context)
    {
      return get_ctx(context, "dt")->_dt;
    }

    unsigned int get_map_id (das::Context* context)
    {
      return get_ctx(context, "get_map_id")->_world->getMapID();
    }

    unsigned int get_area_id(math::vector_3d const& pos, das::Context* context)
    {
      return get_ctx (context, "get_area_id")->_world->getAreaID(pos);
    }
    bool holding_alt (das::Context* context)
    {
      return get_ctx (context, "holding_alt")->_holding_alt;
    }
    bool holding_shift (das::Context* context)
    {
      return get_ctx (context, "holding_shift")->_holding_shift;
    }
    bool holding_ctrl (das::Context* context)
    {
      return get_ctx (context, "holding_ctrl")->_holding_ctrl;
    }
    bool holding_space (das::Context* context)
    {
      return get_ctx (context, "holding_space")->_holding_space;
    }
    math::vector_3d pos (das::Context* context)
    {
      return get_ctx (context, "pos")->_pos;
    }
  } // namespace scripting
} // namespace noggit
