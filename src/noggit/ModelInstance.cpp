// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>
#include <noggit/Log.h>
#include <noggit/Misc.h> // checkinside
#include <noggit/Model.h> // Model, etc.
#include <noggit/ModelInstance.h>
#include <noggit/Settings.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

namespace
{
  math::vector_3d TransformCoordsForModel(math::vector_3d pIn)
  {
    return {pIn.x, pIn.z, -pIn.y};
  }
}

ModelInstance::ModelInstance(std::string const& filename)
  : model (filename)
{
}

ModelInstance::ModelInstance(std::string const& filename, MPQFile* f)
  : model (filename)
{
  float ff[4];

  f->read(ff, 12);
  pos = math::vector_3d(ff[0], ff[2], -ff[1]);

  f->read(ff, 16);
  _wmo_orientation = math::quaternion (-ff[3], ff[1], ff[2], ff[0]);

  f->read(&scale, 4);
  f->read(&uid, 4);
  lcol = math::vector_3d(((uid & 0xff0000) >> 16) / 255.0f, ((uid & 0x00ff00) >> 8) / 255.0f, (uid & 0x0000ff) / 255.0f);
}

ModelInstance::ModelInstance(std::string const& filename, ENTRY_MDDF const*d)
  : model (filename)
{
	uid = d->uniqueID;
	pos = math::vector_3d(d->pos[0], d->pos[1], d->pos[2]);
	dir = math::vector_3d(d->rot[0], d->rot[1], d->rot[2]);
	// scale factor - divide by 1024. blizzard devs must be on crack, why not just use a float?
	scale = d->scale / 1024.0f;

  recalcExtents();
}

void ModelInstance::draw ( math::frustum const& frustum
                         , const float& cull_distance
                         , const math::vector_3d& camera
                         , bool force_box
                         , bool all_boxes
                         , bool draw_fog
                         , bool is_current_selection
                         , int animtime
                         )
{
  float dist = (pos - camera).length() - model->rad * scale;
  if(dist >= cull_distance)
    return;

  if (!frustum.intersectsSphere(pos, model->rad * scale))
    return;

  opengl::scoped::matrix_pusher const matrix;

  math::matrix_4x4 const model_matrix
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                       , { math::degrees (-dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (dir.x)
                         }
                       )
    * math::matrix_4x4 (math::matrix_4x4::scale, scale)
    );

  gl.multMatrixf (model_matrix.transposed());

  if (all_boxes)
  {
    opengl::primitives::wire_box ( TransformCoordsForModel(model->header.VertexBoxMin)
                                 , TransformCoordsForModel(model->header.VertexBoxMax)
                                 ).draw ({0.5f, 0.5f, 0.5f, 1.0f}, 1.0f);
  }
  //! \todo add toggle draw particles and set customizable distance
  model->draw (draw_fog, animtime, dist < 50.0f);

  if (is_current_selection || force_box)
  {
    if (draw_fog)
      gl.disable(GL_FOG);

    gl.disable(GL_LIGHTING);

    gl.disable(GL_COLOR_MATERIAL);
    opengl::texture::set_active_texture (0);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    math::vector_4d color = force_box ? math::vector_4d(0.0f, 0.0f, 1.0f, 1.0f) : math::vector_4d(1.0f, 1.0f, 0.0f, 1.0f);

    opengl::primitives::wire_box ( TransformCoordsForModel(model->header.BoundingBoxMin)
                                 , TransformCoordsForModel(model->header.BoundingBoxMax)
                                 ).draw (color, 1.0f);

    if (is_current_selection)
    {
      opengl::primitives::wire_box ( TransformCoordsForModel(model->header.VertexBoxMin)
                                   , TransformCoordsForModel(model->header.VertexBoxMax)
                                   ).draw ({1.0f, 1.0f, 1.0f, 1.0f}, 1.0f);

      gl.color4fv(math::vector_4d(1.0f, 0.0f, 0.0f, 1.0f));
      gl.begin(GL_LINES);
      gl.vertex3f(0.0f, 0.0f, 0.0f);
      gl.vertex3f(model->header.VertexBoxMax.x + model->header.VertexBoxMax.x / 5.0f, 0.0f, 0.0f);
      gl.end();

      gl.color4fv(math::vector_4d(0.0f, 1.0f, 0.0f, 1.0f));
      gl.begin(GL_LINES);
      gl.vertex3f(0.0f, 0.0f, 0.0f);
      gl.vertex3f(0.0f, model->header.VertexBoxMax.z + model->header.VertexBoxMax.z / 5.0f, 0.0f);
      gl.end();

      gl.color4fv(math::vector_4d(0.0f, 0.0f, 1.0f, 1.0f));
      gl.begin(GL_LINES);
      gl.vertex3f(0.0f, 0.0f, 0.0f);
      gl.vertex3f(0.0f, 0.0f, model->header.VertexBoxMax.y + model->header.VertexBoxMax.y / 5.0f);
      gl.end();
    }

    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture (0);
    opengl::texture::enable_texture();

    gl.enable(GL_LIGHTING);

    if (draw_fog)
      gl.enable(GL_FOG);
  }
}

//! \todo  Get this drawn on the 2D view.
/*void ModelInstance::drawMapTile()
{
if(CheckUniques(uid))
return;

opengl::scoped::matrix_pusher const matrix;

gl.translatef(pos.x/CHUNKSIZE, pos.z/CHUNKSIZE, pos.y);
gl.rotatef(-90.0f, 1, 0, 0);
gl.rotatef(dir.y - 90.0f, 0, 1, 0);
gl.rotatef(-dir.x, 0, 0, 1);
gl.rotatef(dir.z, 1, 0, 0);
gl.scalef(1/CHUNKSIZE,1/CHUNKSIZE,1/CHUNKSIZE);
gl.scalef(sc,sc,sc);

model->draw();
}*/

void ModelInstance::intersect ( math::ray const& ray
                              , selection_result* results
                              , int animtime
                              )
{
  math::matrix_4x4 const model_matrix
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                       , { math::degrees (-dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (dir.x)
                         }
                       )
    * math::matrix_4x4 (math::matrix_4x4::scale, scale)
    );

  math::ray subray (model_matrix.inverted(), ray);

  if ( !subray.intersect_bounds ( fixCoordSystem (model->header.VertexBoxMin)
                                , fixCoordSystem (model->header.VertexBoxMax)
                                )
     )
  {
    return;
  }

  for (auto&& result : model->intersect (subray, animtime))
  {
    //! \todo why is only sc important? these are relative to subray,
    //! so should be inverted by model_matrix?
    results->emplace_back (result * scale, selected_model_type (this));
  }
}


void ModelInstance::draw_wmo ( const math::vector_3d& ofs
                             , const math::degrees rotation
                             , math::frustum const& frustum
                             , bool draw_fog
                             , int animtime
                             )
{
  math::vector_3d tpos(ofs + pos);
  math::rotate (ofs.x, ofs.z, &tpos.x, &tpos.z, rotation);
  if (!frustum.intersectsSphere(tpos, model->rad*scale)) return;

  opengl::scoped::matrix_pusher const matrix;

  gl.translatef(pos.x, pos.y, pos.z);
  gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, _wmo_orientation));
  gl.scalef(scale, -scale, -scale);

  //! \todo draw particles from wmo's doodads ?
  model->draw (draw_fog, animtime, false);
}

void ModelInstance::resetDirection(){
  dir.x = 0;
  //dir.y=0; only reset incline
  dir.z = 0;
}

bool ModelInstance::isInsideRect(math::vector_3d rect[2]) const
{
  return misc::rectOverlap(extents, rect);
}

void ModelInstance::recalcExtents()
{
  math::vector_3d min (math::vector_3d::max()), vertex_box_min (min);
  math::vector_3d max (math::vector_3d::min()), vertex_box_max (max);;
  math::matrix_4x4 rot
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                       , { math::degrees (-dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (dir.x)
                         }
                       )
    * math::matrix_4x4 (math::matrix_4x4::scale, scale)
    );

  math::vector_3d bounds[8 * 2];
  math::vector_3d *ptr = bounds;

  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMax.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMin.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMin.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMax.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMax.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMax.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMax.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMax.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMin.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMax.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.BoundingBoxMin.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMax.z));

  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMax.x, model->header.VertexBoxMax.y, model->header.VertexBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMin.x, model->header.VertexBoxMax.y, model->header.VertexBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMin.x, model->header.VertexBoxMin.y, model->header.VertexBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMax.x, model->header.VertexBoxMin.y, model->header.VertexBoxMin.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMax.x, model->header.VertexBoxMin.y, model->header.VertexBoxMax.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMax.x, model->header.VertexBoxMax.y, model->header.VertexBoxMax.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMin.x, model->header.VertexBoxMax.y, model->header.VertexBoxMax.z));
  *ptr++ = rot * TransformCoordsForModel(math::vector_3d(model->header.VertexBoxMin.x, model->header.VertexBoxMin.y, model->header.VertexBoxMax.z));


  for (int i = 0; i < 8 * 2; ++i)
  {
    misc::extract_v3d_min_max (bounds[i], min, max);
    // vertex box only for size_cat
    if (i >= 8)
    {
      misc::extract_v3d_min_max (bounds[i], vertex_box_min, vertex_box_max);
    }
  }

  extents[0] = min;
  extents[1] = max;

  size_cat = std::max( vertex_box_max.x - vertex_box_min.x
                     , std::max( vertex_box_max.y - vertex_box_min.y
                               , vertex_box_max.z - vertex_box_min.z
                               )
                     );
}