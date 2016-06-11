// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ModelInstance.h>

#include <cassert>
#include <ctime>

#include <math/matrix_4x4.h>

#include <opengl/context.h>
#include <opengl/primitives.h>
#include <opengl/scoped.h>

#include <noggit/Frustum.h> // Frustum
#include <noggit/Log.h>
#include <noggit/Model.h>
#include <noggit/ModelManager.h>
#include <noggit/mpq/file.h>
#include <noggit/World.h>

ModelInstance::ModelInstance (World* world, std::string const& path)
  : model (path)
  , nameID (0xFFFFFFFF)
  , _world (world)
  , _spawn_timestamp (clock() / CLOCKS_PER_SEC)
{
  nameID = _world->selection_names().add (this);
}

ModelInstance::ModelInstance (World* world, std::string const& path, noggit::mpq::file* f)
  : model (path)
  , nameID (0xFFFFFFFF)
  , _world (world)
  , _spawn_timestamp (clock() / CLOCKS_PER_SEC)
{
  f->read (&d1, 4);
  f->read (pos, 12);
  f->read (dir, 12);
  int16_t scale;
  f->read (&scale, sizeof (int16_t));
  sc = scale / 1024.0f;
  nameID = _world->selection_names().add( this );
}

ModelInstance::ModelInstance (World* world, std::string const& path, ENTRY_MDDF *d)
  : model (path)
  , nameID (0xFFFFFFFF)
  , _world (world)
  , _spawn_timestamp (clock() / CLOCKS_PER_SEC)
{
  d1 = d->uniqueID;
  pos = ::math::vector_3d(d->pos[0], d->pos[1], d->pos[2]);
  dir = ::math::vector_3d(d->rot[0], d->rot[1], d->rot[2]);
  sc = d->scale / 1024.0f;
  nameID = _world->selection_names().add( this );
}

ModelInstance::ModelInstance ( World* world
                             , std::string const& path
                             , const ::math::vector_3d& position
                             , const ::math::quaternion& rotation
                             , const float& scale
                             , const ::math::vector_3d& lighting_color
                             )
  : model (path)
  , nameID (0xFFFFFFFF)
  , pos (position)
  , _wmo_doodad_rotation (rotation)
  , sc (scale)
  , lcol (lighting_color)
  , _world(world)
  , _spawn_timestamp (clock() / CLOCKS_PER_SEC)
{ }

ModelInstance::~ModelInstance()
{
  if( nameID != 0xFFFFFFFF )
  {
    LogDebug<<"Delete Item "<<nameID<<std::endl;
    _world->selection_names().del (nameID);
    nameID = 0xFFFFFFFF;
  }
}

void ModelInstance::draw_selection_indicator() const
{
  ::opengl::scoped::bool_setter<GL_FOG, GL_FALSE> fog_setter;
  ::opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting_setter;
  ::opengl::scoped::bool_setter<GL_BLEND, GL_TRUE> blend_setter;
  ::opengl::scoped::bool_setter<GL_COLOR_MATERIAL, GL_FALSE> color_mat_setter;
  ::opengl::scoped::texture_setter<0, GL_FALSE> texture_0_setter;
  ::opengl::scoped::texture_setter<1, GL_FALSE> texture_1_setter;

  gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  static const ::math::vector_4d white (1.0f, 1.0f, 1.0f, 1.0f);
  static const ::math::vector_4d yellow (1.0f, 1.0f, 0.0f, 1.0f);
  static const ::math::vector_4d red (1.0f, 0.0f, 0.0f, 1.0f);
  static const ::math::vector_4d green (0.0f, 1.0f, 0.0f, 1.0f);
  static const ::math::vector_4d blue (0.0f, 0.0f, 1.0f, 1.0f);

  const ::opengl::primitives::wire_box vertex_bounding
    ( fixCoordSystem (model->header.VertexBoxMin)
    , fixCoordSystem (model->header.VertexBoxMax)
    );
  const ::opengl::primitives::wire_box bounding_bounding
    ( fixCoordSystem (model->header.BoundingBoxMin)
    , fixCoordSystem (model->header.BoundingBoxMax)
    );

  vertex_bounding.draw (white, 1.0f);
  bounding_bounding.draw (yellow, 1.0f);

  gl.begin (GL_LINES);

  gl.color4fv (red);
  gl.vertex3f (0.0f, 0.0f, 0.0f);
  gl.vertex3f ( model->header.VertexBoxMax.x()
             + model->header.VertexBoxMax.x() / 5.0f
             , 0.0f
             , 0.0f
             );

  gl.color4fv (green);
  gl.vertex3f (0.0f, 0.0f, 0.0f);
  gl.vertex3f ( 0.0f
             , model->header.VertexBoxMax.z()
             + model->header.VertexBoxMax.z() / 5.0f
             , 0.0f
             );
  //gl.end();

  gl.color4fv (blue);
  gl.vertex3f (0.0f, 0.0f, 0.0f);
  gl.vertex3f ( 0.0f
             , 0.0f
             , model->header.VertexBoxMax.y()
             + model->header.VertexBoxMax.y() / 5.0f
             );

  gl.end();


  gl.begin(GL_TRIANGLE_STRIP);

  gl.color4f(0,255,0,125);

  gl.normal3f(0.0f, -1.0f, 0.0f);
  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  gl.vertex3f (0.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  gl.normal3f(-1.0f, 0.0f, 0.0f);
  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());

  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());

  gl.normal3f(0.0f, 0.0f, -1.0f);
  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  gl.normal3f(0.0f, 0.0f, 1.0f);
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());

  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());

  gl.normal3f(0.0f, 1.0f, 0.0f);
  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  gl.vertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  gl.vertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  gl.end();

  gl.begin(GL_TRIANGLES);

  gl.color4f(0,255,0,125);

  gl.vertex3f(-5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMax.y());
  gl.vertex3f(5.0f, model->header.VertexBoxMin.z(),-model->header.VertexBoxMax.y());
  gl.vertex3f(0.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMax.y() -model->header.VertexBoxMax.y()/2);

  gl.vertex3f(model->header.VertexBoxMin.x(), model->header.VertexBoxMin.z(), 5.0f);
  gl.vertex3f(model->header.VertexBoxMin.x(), model->header.VertexBoxMin.z(),-5.0f);
  gl.vertex3f(model->header.VertexBoxMin.x()+model->header.VertexBoxMin.x()/2, model->header.VertexBoxMin.z(), 0.0f);

  gl.vertex3f(model->header.VertexBoxMax.x(), model->header.VertexBoxMin.z(),-5.0f);
  gl.vertex3f(model->header.VertexBoxMax.x(), model->header.VertexBoxMin.z(), 5.0f);
  gl.vertex3f(model->header.VertexBoxMax.x()+model->header.VertexBoxMax.x()/2, model->header.VertexBoxMin.z(), 0.0f);

  gl.end();


}


bool ModelInstance::is_visible ( const float& cull_distance
                               , const Frustum& frustum
                               , const ::math::vector_3d& camera
                               , const ::math::vector_3d& offset
                               , const float& rotation
                               ) const
{
  const ::math::vector_3d& base_position (pos);
  const float radius (model->rad * sc);

  ::math::vector_3d position (base_position + offset);
  ::math::rotate ( offset.x(), offset.z()
                 , &position.x(), &position.z()
                 , rotation
                 );

  return frustum.intersectsSphere (position, radius)
      && ((position - camera).length() < (cull_distance + radius));
}

size_t ModelInstance::time_since_spawn() const
{
  return (clock() / CLOCKS_PER_SEC) - _spawn_timestamp;
}

void ModelInstance::draw ( bool draw_fog
                         , const boost::optional<selection_type>& selected_item
                         ) const
{
  ::opengl::scoped::matrix_pusher positioning_matrix;

  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, pos).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, convert_rotation (dir)).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::scale, sc));

  model->draw (draw_fog, time_since_spawn());

  const bool is_selected ( selected_item
                        && noggit::selection::is_the_same_as ( this
                                                             , *selected_item
                                                             )
                         );

  if (is_selected)
  {
    draw_selection_indicator();
  }
}

//! \todo  Get this drawn on the 2D view.
/*void ModelInstance::drawMapTile ()
{
  ::opengl::scoped::matrix_pusher positioning_matrix;

  gl.translatef(pos.x/CHUNKSIZE, pos.z/CHUNKSIZE, pos.y);
  gl.rotatef(-90.0f, 1, 0, 0);
  gl.rotatef(dir.y - 90.0f, 0, 1, 0);
  gl.rotatef(-dir.x, 0, 0, 1);
  gl.rotatef(dir.z, 1, 0, 0);
  gl.scalef(1/CHUNKSIZE,1/CHUNKSIZE,1/CHUNKSIZE);
  gl.scalef(sc,sc,sc);

  model->draw ();
}*/

void ModelInstance::draw_for_selection()
{
  if(!_world->selection_names().findEntry(nameID) || nameID == 0xFFFFFFFF)
  {
    nameID = _world->selection_names().add( this );
  }

  ::opengl::scoped::matrix_pusher positioning_matrix;
  ::opengl::scoped::name_pusher name_pusher (nameID);

  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, pos).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, convert_rotation (dir)).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::scale, sc));

  model->drawSelect (time_since_spawn());
}

void ModelInstance::draw2() const
{
  ::opengl::scoped::matrix_pusher positioning_matrix;

  //! \todo This could all be done in one composed matrix.
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, pos).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, _wmo_doodad_rotation));
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::scale, {sc, -sc, -sc}));

  model->draw (_world, time_since_spawn());
}

void ModelInstance::draw2Select() const
{
  ::opengl::scoped::matrix_pusher positioning_matrix;

  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, pos).transposed());
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, _wmo_doodad_rotation));
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::scale, {sc, -sc, -sc}));

  model->drawSelect (time_since_spawn());
}

void ModelInstance::resetDirection()
{
  dir.x (0.0f);
  dir.z (0.0f);
}

namespace
{
  void maybe_expand ( std::pair<::math::vector_3d, ::math::vector_3d>& extents
                    , ::math::vector_3d new_point
                    )
  {
    extents.first = ::math::min (extents.first, new_point);
    extents.second = ::math::max (extents.second, new_point);
  }
}

std::pair<::math::vector_3d, ::math::vector_3d> ModelInstance::extents() const
{
  ::math::matrix_4x4 const rot
    ( ::math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * ::math::matrix_4x4 (math::matrix_4x4::rotation, convert_rotation (dir))
    * ::math::matrix_4x4 (math::matrix_4x4::scale, sc)
    );

  std::pair<::math::vector_3d, ::math::vector_3d> extents
    (::math::vector_3d::max(), ::math::vector_3d::min());

  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMin.x(), model->header.VertexBoxMin.y(), model->header.VertexBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMin.x(), model->header.VertexBoxMin.y(), model->header.VertexBoxMax.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMin.x(), model->header.VertexBoxMax.y(), model->header.VertexBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMin.x(), model->header.VertexBoxMax.y(), model->header.VertexBoxMax.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMax.x(), model->header.VertexBoxMin.y(), model->header.VertexBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMax.x(), model->header.VertexBoxMin.y(), model->header.VertexBoxMax.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMax.x(), model->header.VertexBoxMax.y(), model->header.VertexBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.VertexBoxMax.x(), model->header.VertexBoxMax.y(), model->header.VertexBoxMax.z()}));

  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMin.x(), model->header.BoundingBoxMin.y(), model->header.BoundingBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMin.x(), model->header.BoundingBoxMin.y(), model->header.BoundingBoxMax.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMin.x(), model->header.BoundingBoxMax.y(), model->header.BoundingBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMin.x(), model->header.BoundingBoxMax.y(), model->header.BoundingBoxMax.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMax.x(), model->header.BoundingBoxMin.y(), model->header.BoundingBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMax.x(), model->header.BoundingBoxMin.y(), model->header.BoundingBoxMax.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMax.x(), model->header.BoundingBoxMax.y(), model->header.BoundingBoxMin.z()}));
  maybe_expand (extents, rot * fixCoordSystem ({model->header.BoundingBoxMax.x(), model->header.BoundingBoxMax.y(), model->header.BoundingBoxMax.z()}));

  return extents;
}
