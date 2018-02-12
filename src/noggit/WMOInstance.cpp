// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/Misc.h> // checkinside
#include <noggit/ModelInstance.h>
#include <noggit/WMO.h> // WMO
#include <noggit/WMOInstance.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

WMOInstance::WMOInstance(std::string const& filename, ENTRY_MODF const* d)
  : wmo(filename)
  , pos(math::vector_3d(d->pos[0], d->pos[1], d->pos[2]))
  , dir(math::vector_3d(d->rot[0], d->rot[1], d->rot[2]))
  , mUniqueID(d->uniqueID), mFlags(d->flags)
  , mUnknown(d->unknown), mNameset(d->nameSet)
  , _doodadset(d->doodadSet)
{
  extents[0] = math::vector_3d(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
  extents[1] = math::vector_3d(d->extents[1][0], d->extents[1][1], d->extents[1][2]);

  change_doodadset(_doodadset);
}

WMOInstance::WMOInstance(std::string const& filename)
  : wmo(filename)
  , pos(math::vector_3d(0.0f, 0.0f, 0.0f))
  , dir(math::vector_3d(0.0f, 0.0f, 0.0f))
  , mUniqueID(0)
  , mFlags(0)
  , mUnknown(0)
  , mNameset(0)
  , _doodadset(0)
{
  change_doodadset(_doodadset);
}

void WMOInstance::draw ( math::frustum const& frustum
                       , const float& cull_distance
                       , const math::vector_3d& camera
                       , bool force_box
                       , bool draw_doodads
                       , bool draw_fog
                       , math::vector_4d const& ocean_color_light
                       , math::vector_4d const& ocean_color_dark
                       , math::vector_4d const& river_color_light
                       , math::vector_4d const& river_color_dark
                       , liquid_render& render
                       , boost::optional<selection_type> selection
                       , int animtime
                       , std::function<void (bool)> setup_outdoor_lights
                       , bool world_has_skies
                       , std::function<void (bool)> setup_fog
                       )
{
  bool const is_selected
    ( selection
    && boost::get<selected_wmo_type> (&*selection)
    && boost::get<selected_wmo_type> (*selection)->mUniqueID == this->mUniqueID
    );

  {
    opengl::scoped::matrix_pusher const matrix;
    gl.translatef(pos.x, pos.y, pos.z);

    const float roty = dir.y - 90.0f;

    gl.rotatef(roty, 0.0f, 1.0f, 0.0f);
    gl.rotatef(-dir.x, 0.0f, 0.0f, 1.0f);
    gl.rotatef(dir.z, 1.0f, 0.0f, 0.0f);

    wmo->draw ( _doodadset
              , pos
              , math::degrees (roty)
              , is_selected
              , frustum
              , cull_distance
              , camera
              , draw_doodads
              , draw_fog
              , ocean_color_light
              , ocean_color_dark
              , river_color_light
              , river_color_dark
              , render
              , animtime
              , setup_outdoor_lights
              , world_has_skies
              , setup_fog
              );
  }

  if (force_box || is_selected)
  {
    gl.disable(GL_LIGHTING);

    gl.disable(GL_COLOR_MATERIAL);
    opengl::texture::disable_texture(0);
    opengl::texture::disable_texture(1);
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    math::vector_4d color = force_box ? math::vector_4d(0.0f, 0.0f, 1.0f, 1.0f) : math::vector_4d(0.0f, 1.0f, 0.0f, 1.0f);
    opengl::primitives::wire_box (extents[0], extents[1]).draw (color, 1.0f);

    opengl::texture::disable_texture(1);
    opengl::texture::enable_texture(0);

    gl.enable(GL_LIGHTING);
  }
}

void WMOInstance::intersect (math::ray const& ray, selection_result* results)
{
  if (!ray.intersect_bounds (extents[0], extents[1]))
  {
    return;
  }

  math::matrix_4x4 const model_matrix
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                       , { math::degrees (-dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (dir.x)
                         }
                       )
    );

  for (auto&& result : wmo->intersect ({model_matrix.inverted(), ray}))
  {
    results->emplace_back (result, selected_wmo_type (this));
  }
}

void WMOInstance::recalcExtents()
{
  update_doodads();

  math::vector_3d min (math::vector_3d::max());
  math::vector_3d max (math::vector_3d::min());
  math::matrix_4x4 rot
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                       , { math::degrees (-dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (dir.x)
                         }
                       )
    );

  std::vector<math::vector_3d> bounds (8 * (wmo->groups.size() + 1));
  math::vector_3d *ptr = bounds.data();
  math::vector_3d wmoMin(wmo->extents[0].x, wmo->extents[0].z, -wmo->extents[0].y);
  math::vector_3d wmoMax(wmo->extents[1].x, wmo->extents[1].z, -wmo->extents[1].y);

  *ptr++ = rot * math::vector_3d(wmoMax.x, wmoMax.y, wmoMin.z);
  *ptr++ = rot * math::vector_3d(wmoMin.x, wmoMax.y, wmoMin.z);
  *ptr++ = rot * math::vector_3d(wmoMin.x, wmoMin.y, wmoMin.z);
  *ptr++ = rot * math::vector_3d(wmoMax.x, wmoMin.y, wmoMin.z);
  *ptr++ = rot * math::vector_3d(wmoMax.x, wmoMin.y, wmoMax.z);
  *ptr++ = rot * math::vector_3d(wmoMax.x, wmoMax.y, wmoMax.z);
  *ptr++ = rot * math::vector_3d(wmoMin.x, wmoMax.y, wmoMax.z);
  *ptr++ = rot * math::vector_3d(wmoMin.x, wmoMin.y, wmoMax.z);

  for (int i = 0; i < (int)wmo->groups.size(); ++i)
  {
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * math::vector_3d(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMax.z);
  }

  for (int i = 0; i < 8 * ((int)wmo->groups.size() + 1); ++i)
  {
    misc::extract_v3d_min_max (bounds[i], min, max);
  }

  extents[0] = min;
  extents[1] = max;
}

bool WMOInstance::isInsideRect(math::vector_3d rect[2]) const
{
  return misc::rectOverlap(extents, rect);
}

void WMOInstance::change_doodadset(uint16_t doodad_set)
{
  _doodadset = doodad_set;

  _doodads_per_group = wmo->doodads_per_group(_doodadset);
  update_doodads();
}

void WMOInstance::update_doodads()
{
  for (auto& group_doodads : _doodads_per_group)
  {
    for (auto& doodad : group_doodads.second)
    {
      doodad.update_transform_matrix_wmo(this);
    }
  }
}

void WMOInstance::resetDirection()
{
  dir = math::vector_3d(0.0f, dir.y, 0.0f);
  recalcExtents();

}

std::vector<wmo_doodad_instance*> WMOInstance::get_visible_doodads
  ( math::frustum const& frustum
  , float const& cull_distance
  , math::vector_3d const& camera
  , bool draw_hidden_models
  )
{
  std::vector<wmo_doodad_instance*> doodads;

  if (!wmo->is_hidden() || draw_hidden_models)
  {
    for (int i = 0; i < wmo->groups.size(); ++i)
    {
      if (wmo->groups[i].is_visible(pos, math::degrees (dir.y - 90.f), frustum, cull_distance, camera))
      {
        for (auto& doodad : _doodads_per_group[i])
        {
          doodads.push_back(&doodad);
        }
      }
    }
  } 

  return doodads;
}
