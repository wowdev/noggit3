// ModelInstance.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/ModelInstance.h>

#include <cassert>

#include <math/matrix_4x4.h>

#include <opengl/primitives.h>
#include <opengl/scoped.h>

#include <noggit/Log.h>
#include <noggit/Model.h>
#include <noggit/World.h>
#include <noggit/mpq/file.h>

ModelInstance::ModelInstance (World* world, Model *m)
  : model (m)
  , _world (world)
  , nameID (0xFFFFFFFF)
{
  nameID = _world->selection_names().add (this);
}

ModelInstance::ModelInstance (World* world, Model *m, noggit::mpq::file* f)
  : model (m)
  , _world (world)
  , nameID (0xFFFFFFFF)
{
  f->read (&d1, 4);
  f->read (pos, 12);
  f->read (dir, 12);
  int16_t scale;
  f->read (&scale, sizeof (int16_t));
  sc = scale / 1024.0f;
  nameID = _world->selection_names().add( this );
}

ModelInstance::ModelInstance (World* world, Model *m, ENTRY_MDDF *d)
  : model (m)
  , _world (world)
  , nameID (0xFFFFFFFF)
{
  d1 = d->uniqueID;
  pos = ::math::vector_3d(d->pos[0], d->pos[1], d->pos[2]);
  dir = ::math::vector_3d(d->rot[0], d->rot[1], d->rot[2]);
  sc = d->scale / 1024.0f;
  nameID = _world->selection_names().add( this );
}

ModelInstance::ModelInstance ( World* world
                             , Model* model
                             , const ::math::vector_3d& position
                             , const ::math::quaternion& rotation
                             , const float& scale
                             , const ::math::vector_3d& lighting_color
                             )
  : model (model)
  , nameID (0xFFFFFFFF)
  , pos (position)
  , _wmo_doodad_rotation (rotation)
  , sc (scale)
  , lcol (lighting_color)
  , _world(world)
{
  nameID = _world->selection_names().add (this);
}

ModelInstance::~ModelInstance()
{
  if( nameID != 0xFFFFFFFF )
  {
    LogDebug<<"Delete Item "<<nameID<<std::endl;
    _world->selection_names().del (nameID);
    nameID = 0xFFFFFFFF;
  }
}

::math::vector_3d TransformCoordsForModel (const ::math::vector_3d& pIn)
{
  ::math::vector_3d lTemp (pIn);
  lTemp.y (pIn.z());
  lTemp.z (-pIn.y());
  return lTemp;
}

static const float doodaddrawdistance2 (700.0f * 700.0f);

#define MAYBE_DONT_DRAW \
  ::math::vector_3d tpos (ofs + pos); \
  ::math::rotate (ofs.x(), ofs.z(), &tpos.x(), &tpos.z(), rot); \
  if ( (tpos - _world->camera).length_squared() > (doodaddrawdistance2 * model->rad * sc) || !frustum.intersectsSphere (tpos, model->rad * sc)) return


void ModelInstance::draw_selection_indicator() const
{
  ::opengl::scoped::bool_setter<GL_FOG, GL_FALSE> fog_setter;
  ::opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting_setter;
  ::opengl::scoped::bool_setter<GL_BLEND, GL_TRUE> blend_setter;
  ::opengl::scoped::bool_setter<GL_COLOR_MATERIAL, GL_FALSE> color_mat_setter;
  ::opengl::scoped::texture_setter<GL_TEXTURE0, GL_FALSE> texture_0_setter;
  ::opengl::scoped::texture_setter<GL_TEXTURE1, GL_FALSE> texture_1_setter;

  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  static const ::math::vector_4d white (1.0f, 1.0f, 1.0f, 1.0f);
  static const ::math::vector_4d yellow (1.0f, 1.0f, 0.0f, 1.0f);
  static const ::math::vector_4d red (1.0f, 0.0f, 0.0f, 1.0f);
  static const ::math::vector_4d green (0.0f, 1.0f, 0.0f, 1.0f);
  static const ::math::vector_4d blue (0.0f, 0.0f, 1.0f, 1.0f);

  const ::opengl::primitives::wire_box vertex_bounding
    ( TransformCoordsForModel (model->header.VertexBoxMin)
    , TransformCoordsForModel (model->header.VertexBoxMax)
    );
  const ::opengl::primitives::wire_box bounding_bounding
    ( TransformCoordsForModel (model->header.BoundingBoxMin)
    , TransformCoordsForModel (model->header.BoundingBoxMax)
    );

  vertex_bounding.draw (white, 1.0f);
  bounding_bounding.draw (yellow, 1.0f);

  glBegin (GL_LINES);

  glColor4fv (red);
  glVertex3f (0.0f, 0.0f, 0.0f);
  glVertex3f ( model->header.VertexBoxMax.x()
             + model->header.VertexBoxMax.x() / 5.0f
             , 0.0f
             , 0.0f
             );

  glColor4fv (green);
  glVertex3f (0.0f, 0.0f, 0.0f);
  glVertex3f ( 0.0f
             , model->header.VertexBoxMax.z()
             + model->header.VertexBoxMax.z() / 5.0f
             , 0.0f
             );
  //glEnd();

  glColor4fv (blue);
  glVertex3f (0.0f, 0.0f, 0.0f);
  glVertex3f ( 0.0f
             , 0.0f
             , model->header.VertexBoxMax.y()
             + model->header.VertexBoxMax.y() / 5.0f
             );

  glEnd();


  glBegin(GL_TRIANGLE_STRIP);

  glColor4f(0,255,0,125);

  glNormal3f(0.0f, -1.0f, 0.0f);
  glVertex3f(-5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  glVertex3f( 5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  glVertex3f (0.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  glNormal3f(-1.0f, 0.0f, 0.0f);
  glVertex3f(-5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  glVertex3f( 5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMin.y());
  glVertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());

  glVertex3f(-5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  glVertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  glVertex3f(-5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());

  glNormal3f(0.0f, 0.0f, -1.0f);
  glVertex3f(-5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  glVertex3f(-5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  glVertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  glVertex3f(-5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  glVertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  glVertex3f( 0.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  glNormal3f(0.0f, 0.0f, 1.0f);
  glVertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  glVertex3f( 0.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  glVertex3f( 5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());

  glVertex3f( 5.0f, model->header.VertexBoxMin.z(),      -model->header.VertexBoxMin.y());
  glVertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);
  glVertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());

  glNormal3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  glVertex3f( 5.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y());
  glVertex3f( 0.0f, model->header.VertexBoxMin.z()+2.0f, -model->header.VertexBoxMin.y()+(-model->header.VertexBoxMin.y())/2);

  glEnd();

  glBegin(GL_TRIANGLES);

  glColor4f(0,255,0,125);

  glVertex3f(-5.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMax.y());
  glVertex3f(5.0f, model->header.VertexBoxMin.z(),-model->header.VertexBoxMax.y());
  glVertex3f(0.0f, model->header.VertexBoxMin.z(), -model->header.VertexBoxMax.y() -model->header.VertexBoxMax.y()/2);

  glVertex3f(model->header.VertexBoxMin.x(), model->header.VertexBoxMin.z(), 5.0f);
  glVertex3f(model->header.VertexBoxMin.x(), model->header.VertexBoxMin.z(),-5.0f);
  glVertex3f(model->header.VertexBoxMin.x()+model->header.VertexBoxMin.x()/2, model->header.VertexBoxMin.z(), 0.0f);

  glVertex3f(model->header.VertexBoxMax.x(), model->header.VertexBoxMin.z(),-5.0f);
  glVertex3f(model->header.VertexBoxMax.x(), model->header.VertexBoxMin.z(), 5.0f);
  glVertex3f(model->header.VertexBoxMax.x()+model->header.VertexBoxMax.x()/2, model->header.VertexBoxMin.z(), 0.0f);

  glEnd();


}

void ModelInstance::draw (bool draw_fog, const Frustum& frustum)
{
  static const ::math::vector_3d ofs (0.0f, 0.0f, 0.0f);
  static const float rot (0.0);
  MAYBE_DONT_DRAW;

  ::opengl::scoped::matrix_pusher positioning_matrix;

  glTranslatef (pos.x(), pos.y(), pos.z());
  glRotatef (dir.y() - 90.0f, 0.0f, 1.0f, 0.0f);
  glRotatef (-dir.x(), 0.0f, 0.0f, 1.0f);
  glRotatef (dir.z(), 1.0f, 0.0f, 0.0f);
  glScalef (sc, sc, sc);

  model->draw (draw_fog);

  const bool is_selected ( _world->IsSelection (eEntry_Model)
                        && _world->GetCurrentSelection()->data.model->d1 == d1
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

  glTranslatef(pos.x/CHUNKSIZE, pos.z/CHUNKSIZE, pos.y);
  glRotatef(-90.0f, 1, 0, 0);
  glRotatef(dir.y - 90.0f, 0, 1, 0);
  glRotatef(-dir.x, 0, 0, 1);
  glRotatef(dir.z, 1, 0, 0);
  glScalef(1/CHUNKSIZE,1/CHUNKSIZE,1/CHUNKSIZE);
  glScalef(sc,sc,sc);

  model->draw ();
}*/

void ModelInstance::draw_for_selection(const Frustum& frustum)
{
  static const ::math::vector_3d ofs (0.0f, 0.0f, 0.0f);
  static const float rot (0.0);
  MAYBE_DONT_DRAW;

  if(!_world->selection_names().findEntry(nameID) || nameID == 0xFFFFFFFF)
  {
    LogDebug<<"Old item" << nameID << "not found create new one"<<std::endl;
    nameID = _world->selection_names().add( this );
  }

  ::opengl::scoped::matrix_pusher positioning_matrix;
  ::opengl::scoped::name_pusher name_pusher (nameID);

  LogDebug << "I am  " << nameID << std::endl;

  glTranslatef( pos.x(), pos.y(), pos.z() );
  glRotatef( dir.y() - 90.0f, 0.0f, 1.0f, 0.0f );
  glRotatef( -dir.x(), 0.0f, 0.0f, 1.0f );
  glRotatef( dir.z(), 1.0f, 0.0f, 0.0f );
  glScalef( sc, sc, sc );

  model->drawSelect();
}

void ModelInstance::draw2 ( const ::math::vector_3d& ofs
                          , const float rot
                          , const Frustum& frustum
                          )
{
  MAYBE_DONT_DRAW;

  ::opengl::scoped::matrix_pusher positioning_matrix;

  //! \todo This could all be done in one composed matrix.
  glTranslatef (pos.x(), pos.y(), pos.z());
  glMultMatrixf (::math::matrix_4x4::new_rotation_matrix (_wmo_doodad_rotation));
  glScalef (sc, -sc, -sc);

  model->draw (_world);
}

void ModelInstance::draw2Select ( const ::math::vector_3d& ofs
                                , const float rot
                                , const Frustum& frustum
                                )
{
  MAYBE_DONT_DRAW;

  ::opengl::scoped::matrix_pusher positioning_matrix;

  glTranslatef (pos.x(), pos.y(), pos.z());
  glMultMatrixf (::math::matrix_4x4::new_rotation_matrix (_wmo_doodad_rotation));
  glScalef (sc, -sc, -sc);

  model->drawSelect();
}

void ModelInstance::resetDirection()
{
  dir.x (0.0f);
  dir.z (0.0f);
}
