// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/WMOInstance.h>

#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/WMO.h> // WMO
#include <noggit/World.h>
#include <noggit/mpq/file.h>

#include <opengl/context.h>
#include <opengl/scoped.h>
#include <opengl/matrix.h>
#include <opengl/shader.hpp>

WMOInstance::WMOInstance(World* world, std::string const& path, noggit::mpq::file* _file )
  : wmo (world, path)
  , _world (world)
{
  _file->read( &mUniqueID, 4 );
  _file->read( &pos, 12 );
  _file->read( &dir, 12 );
  _file->read( &extents[0], 12 );
  _file->read( &extents[1], 12 );
  _file->read( &mFlags, 2 );
  _file->read( &doodadset, 2 );
  _file->read( &mNameset, 2 );
  _file->read( &mUnknown, 2 );
}

WMOInstance::WMOInstance( World* world, std::string const& path, ENTRY_MODF* d )
  : wmo (world, path)
  , pos( d->pos[0], d->pos[1], d->pos[2] )
  , dir( d->rot[0], d->rot[1], d->rot[2] )
  , mUniqueID( d->uniqueID )
  , mFlags( d->flags )
  , mUnknown( d->unknown )
  , mNameset( d->nameSet )
  , doodadset( d->doodadSet )
  , _world (world)
{
  extents[0] = ::math::vector_3d( d->extents[0][0], d->extents[0][1], d->extents[0][2] );
  extents[1] = ::math::vector_3d( d->extents[1][0], d->extents[1][1], d->extents[1][2] );
}

WMOInstance::WMOInstance( World* world, std::string const& path )
  : wmo (world, path)
  , pos( 0.0f, 0.0f, 0.0f )
  , dir( 0.0f, 0.0f, 0.0f )
  , mUniqueID( 0 )
  , mFlags( 0 )
  , mUnknown( 0 )
  , mNameset( 0 )
  , doodadset( 0 )
  , _world (world)
{
}

void WMOInstance::draw ( opengl::scoped::use_program& shader
                       , bool draw_fog
                       , bool hasSkies
                       , const float culldistance
                       , const float& fog_distance
                       , const Frustum& frustum
                       , const ::math::vector_3d& camera
                       , const boost::optional<selection_type>& selected_item
                       ) const
{
  opengl::scoped::matrix_pusher const matrix_pusher;

  math::matrix_4x4 const translation (math::matrix_4x4::translation, pos);
  math::matrix_4x4 const rotation (math::matrix_4x4::rotation, convert_rotation (dir));

  math::matrix_4x4 const model_matrix (translation * rotation);

  gl.multMatrixf (model_matrix.transposed ());

  const bool is_selected ( selected_item
                        && noggit::selection::is_the_same_as ( this
                                                             , *selected_item
                                                             )
                         );
  shader.uniform ("model_view", opengl::matrix::model_view ());

  wmo->draw ( shader
            , _world
            , pos
            , dir.y() - 90.0f
            , culldistance
            , is_selected
            , is_selected
            , is_selected
            , draw_fog
            , hasSkies
            , fog_distance
            , frustum
            , camera
            );
}

void WMOInstance::draw_doodads( bool draw_fog
                              , const float culldistance
                              , const float& fog_distance
                              , const Frustum& frustum
                              , const ::math::vector_3d& camera
                              )
{
  opengl::scoped::matrix_pusher const matrix_pusher;

  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, pos).transposed ());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, convert_rotation (dir)).transposed ());

  wmo->draw_doodads ( _world
                    , doodadset
                    , pos
                    , dir.y() - 90.0f
                    , culldistance
                    , false
                    , draw_fog
                    , fog_distance
                    , frustum
                    , camera
                    );

}

void WMOInstance::intersect(math::ray ray, selection_result& results)
{
  math::matrix_4x4 const translation (math::matrix_4x4::translation, pos);
  math::matrix_4x4 const rotation (math::matrix_4x4::rotation, convert_rotation (dir));

  math::matrix_4x4 const model_matrix ((translation * rotation).inverted ());

  math::vector_3d const min (extents[0]);
  math::vector_3d const max (extents[1]);

  if (!ray.intersect_bounds (min, max))
    return;

  std::vector<float> subresults;

  wmo->intersect (math::ray (model_matrix, ray), subresults);

  for (float result : subresults)
  {
    results.emplace_back (result, selected_model_type (this));
  }
}

/*void WMOInstance::drawPortals()
{
  opengl::scoped::matrix_pusher const matrix_pusher;

  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, pos).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, convert_rotation (dir)).transposed());

  wmo->drawPortals();
}*/

void WMOInstance::resetDirection()
{
  dir = ::math::vector_3d( 0.0f, dir.y(), 0.0f );
}

WMOInstance::~WMOInstance()
{
}

namespace
{
  void maybe_expand (::math::vector_3d* extents, ::math::vector_3d new_point)
  {
    extents[0] = ::math::min (extents[0], new_point);
    extents[1] = ::math::max (extents[1], new_point);
  }
}

void WMOInstance::recalc_extents()
{
  ::math::matrix_4x4 const rot
    ( ::math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * ::math::matrix_4x4 (math::matrix_4x4::rotation, convert_rotation (dir))
    );

  extents[0] = ::math::vector_3d::max();
  extents[1] = ::math::vector_3d::min();

  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[0].x(), wmo->extents[0].y(), wmo->extents[0].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[0].x(), wmo->extents[0].y(), wmo->extents[1].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[0].x(), wmo->extents[1].y(), wmo->extents[0].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[0].x(), wmo->extents[1].y(), wmo->extents[1].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[1].x(), wmo->extents[0].y(), wmo->extents[0].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[1].x(), wmo->extents[0].y(), wmo->extents[1].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[1].x(), wmo->extents[1].y(), wmo->extents[0].z()));
  maybe_expand (extents, rot * ::math::vector_3d (wmo->extents[1].x(), wmo->extents[1].y(), wmo->extents[1].z()));

  for (std::size_t i (0); i < wmo->nGroups; ++i)
  {
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMin.x(), wmo->groups[i].BoundingBoxMin.y(), wmo->groups[i].BoundingBoxMin.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMin.x(), wmo->groups[i].BoundingBoxMin.y(), wmo->groups[i].BoundingBoxMax.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMin.x(), wmo->groups[i].BoundingBoxMax.y(), wmo->groups[i].BoundingBoxMin.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMin.x(), wmo->groups[i].BoundingBoxMax.y(), wmo->groups[i].BoundingBoxMax.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMax.x(), wmo->groups[i].BoundingBoxMin.y(), wmo->groups[i].BoundingBoxMin.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMax.x(), wmo->groups[i].BoundingBoxMin.y(), wmo->groups[i].BoundingBoxMax.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMax.x(), wmo->groups[i].BoundingBoxMax.y(), wmo->groups[i].BoundingBoxMin.z()));
    maybe_expand (extents, rot * ::math::vector_3d (wmo->groups[i].BoundingBoxMax.x(), wmo->groups[i].BoundingBoxMax.y(), wmo->groups[i].BoundingBoxMax.z()));
  }
}
