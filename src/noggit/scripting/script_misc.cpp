// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <daScript/daScript.h> // must be on top

#include <noggit/scripting/script_context.hpp>
#include <noggit/World.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/camera.hpp>
#include <noggit/scripting/script_misc.hpp>

#include <lodepng.h>

namespace noggit
{
  namespace scripting
  {
    float cam_pitch()
    {
      return get_ctx()->_camera->pitch()._;
    }

    float outer_radius()
    {
      return get_ctx()->_outer_radius;
    }

    float inner_radius()
    {
      return get_ctx()->_inner_radius;
    }

    float cam_yaw()
    {
      return get_ctx()->_camera->yaw()._;
    }

    math::vector_3d vec(float x, float y, float z)
    {
      return math::vector_3d(x, y, z);
    }

    void add_m2(const char *filename, math::vector_3d &pos, float scale, math::vector_3d &rotation)
    {
      auto p = object_paste_params();
      get_ctx()->_world->addM2(filename, pos, scale, rotation, &p);
    }

    void add_wmo(const char *filename, math::vector_3d &pos, math::vector_3d &rotation)
    {
      get_ctx()->_world->addWMO(filename, pos, rotation);
    }

    unsigned int get_map_id()
    {
      return get_ctx()->_world->getMapID();
    }

    unsigned int get_area_id(math::vector_3d &pos)
    {
      return get_ctx()->_world->getAreaID(pos);
    }
    bool holding_alt()
    {
      return get_ctx()->_holding_alt;
    }
    bool holding_shift()
    {
      return get_ctx()->_holding_shift;
    }
    bool holding_ctrl()
    {
      return get_ctx()->_holding_ctrl;
    }
    bool holding_space()
    {
      return get_ctx()->_holding_space;
    }
    math::vector_3d pos()
    {
      return get_ctx()->_pos;
    }
  } // namespace scripting
} // namespace noggit