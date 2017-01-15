// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ModelInstance.h"

#include "Frustum.h"
#include "Log.h"
#include "Misc.h" // checkinside
#include "Model.h" // Model, etc.
#include "World.h" // gWorld
#include "Settings.h" // gWorld
#include <opengl/scoped.hpp>

math::vector_3d TransformCoordsForModel(math::vector_3d pIn)
{
	math::vector_3d lTemp = pIn;
	lTemp.y = pIn.z;
	lTemp.z = -pIn.y;
	return lTemp;
}

void DrawABox(math::vector_3d pMin, math::vector_3d pMax, math::vector_4d pColor, float pLineWidth)
{
	gl.enable(GL_LINE_SMOOTH);
	gl.lineWidth(pLineWidth);
	gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	gl.color4fv(pColor);

	gl.begin(GL_LINE_STRIP);
	gl.vertex3f(pMin.x, pMax.y, pMin.z);
	gl.vertex3f(pMin.x, pMin.y, pMin.z);
	gl.vertex3f(pMax.x, pMin.y, pMin.z);
	gl.vertex3f(pMax.x, pMin.y, pMax.z);
	gl.vertex3f(pMax.x, pMax.y, pMax.z);
	gl.vertex3f(pMax.x, pMax.y, pMin.z);
	gl.vertex3f(pMin.x, pMax.y, pMin.z);
	gl.vertex3f(pMin.x, pMax.y, pMax.z);
	gl.vertex3f(pMin.x, pMin.y, pMax.z);
	gl.vertex3f(pMin.x, pMin.y, pMin.z);
	gl.end();
	gl.begin(GL_LINES);
	gl.vertex3f(pMin.x, pMin.y, pMax.z);
	gl.vertex3f(pMax.x, pMin.y, pMax.z);
	gl.end();
	gl.begin(GL_LINES);
	gl.vertex3f(pMax.x, pMax.y, pMin.z);
	gl.vertex3f(pMax.x, pMin.y, pMin.z);
	gl.end();
	gl.begin(GL_LINES);
	gl.vertex3f(pMin.x, pMax.y, pMax.z);
	gl.vertex3f(pMax.x, pMax.y, pMax.z);
	gl.end();
}

ModelInstance::ModelInstance(std::string const& filename)
	: model (filename)
	, uidLock(false)
{
}

ModelInstance::ModelInstance(std::string const& filename, MPQFile* f)
	: model (filename)
	, uidLock(false)
{
	float ff[3], temp;
	f->read(ff, 12);
	pos = math::vector_3d(ff[0], ff[1], ff[2]);
	temp = pos.z;
	pos.z = -pos.y;
	pos.y = temp;
	f->read(&w, 4);
	f->read(ff, 12);
	dir = math::vector_3d(ff[0], ff[1], ff[2]);
	f->read(&sc, 4);
	f->read(&d1, 4);
	lcol = math::vector_3d(((d1 & 0xff0000) >> 16) / 255.0f, ((d1 & 0x00ff00) >> 8) / 255.0f, (d1 & 0x0000ff) / 255.0f);
}

ModelInstance::ModelInstance(std::string const& filename, ENTRY_MDDF *d)
	: model (filename)
	, uidLock(false)
{
	d1 = d->uniqueID;
	pos = math::vector_3d(d->pos[0], d->pos[1], d->pos[2]);
	dir = math::vector_3d(d->rot[0], d->rot[1], d->rot[2]);
	// scale factor - divide by 1024. blizzard devs must be on crack, why not just use a float?
	sc = d->scale / 1024.0f;
}

void ModelInstance::draw (Frustum const& frustum)
{
	/*  float dist = ( pos - gWorld->camera ).length() - model->rad;

	if( dist > 2.0f * gWorld->modeldrawdistance )
	return;
	if( CheckUniques( d1 ) )
	return;*/

	if (!frustum.intersectsSphere(pos, model->rad * sc))
		return;

  opengl::scoped::matrix_pusher const matrix;

	gl.translatef(pos.x, pos.y, pos.z);
	gl.rotatef(dir.y - 90.0f, 0.0f, 1.0f, 0.0f);
	gl.rotatef(-dir.x, 0.0f, 0.0f, 1.0f);
	gl.rotatef(dir.z, 1.0f, 0.0f, 0.0f);
	gl.scalef(sc, sc, sc);

	if (Settings::getInstance()->renderModelsWithBox)
	{
		gl.disable(GL_LIGHTING);
		gl.disable(GL_COLOR_MATERIAL);
		opengl::texture::set_active_texture (0);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		gl.enable(GL_BLEND);
		gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		DrawABox(TransformCoordsForModel(model->header.VertexBoxMin), TransformCoordsForModel(model->header.VertexBoxMax), math::vector_4d(0.5f, 0.5f, 0.5f, 1.0f), 3.0f);
		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (0);
		opengl::texture::enable_texture();
		gl.enable(GL_LIGHTING);
	}
		model->draw();

  bool currentSelection = gWorld->IsSelection(eEntry_Model) && boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->d1 == d1;

  // no need to check Environment::showModelFromHiddenList as it's done beforehand in World::draw()
	if (currentSelection || model->hidden)
	{
		if (gWorld && gWorld->drawfog)
			gl.disable(GL_FOG);

		gl.disable(GL_LIGHTING);

		gl.disable(GL_COLOR_MATERIAL);
		opengl::texture::set_active_texture (0);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		gl.enable(GL_BLEND);
		gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    math::vector_4d color = model->hidden ? math::vector_4d(0.0f, 0.0f, 1.0f, 1.0f) : math::vector_4d(1.0f, 1.0f, 0.0f, 1.0f);

    DrawABox(TransformCoordsForModel(model->header.BoundingBoxMin), TransformCoordsForModel(model->header.BoundingBoxMax), color, 1.0f);

    if (currentSelection)
    {
      DrawABox(TransformCoordsForModel(model->header.VertexBoxMin), TransformCoordsForModel(model->header.VertexBoxMax), math::vector_4d(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

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

		if (gWorld && gWorld->drawfog)
			gl.enable(GL_FOG);
	}
}

//! \todo  Get this drawn on the 2D view.
/*void ModelInstance::drawMapTile()
{
if(CheckUniques(d1))
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

void ModelInstance::intersect (math::ray const& ray, selection_result* results)
{
  math::matrix_4x4 const model_matrix
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation
                       , { math::degrees (dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (-dir.x)
                         }
                       )
    * math::matrix_4x4 (math::matrix_4x4::scale, sc)
    );

  math::ray subray (model_matrix.inverted(), ray);

  if ( !subray.intersect_bounds ( fixCoordSystem (model->header.VertexBoxMin)
                                , fixCoordSystem (model->header.VertexBoxMax)
                                )
     )
  {
    return;
  }

  for (auto&& result : model->intersect (subray))
  {
    //! \todo why is only sc important? these are relative to subray,
    //! so should be inverted by model_matrix?
    results->emplace_back (result * sc, selected_model_type (this));
  }
}

void quaternionRotate(const math::vector_3d& vdir, float w)
{
	gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, math::quaternion (vdir, w)));
}

void ModelInstance::draw2(const math::vector_3d& ofs, const math::degrees rotation, Frustum const& frustum)
{
	math::vector_3d tpos(ofs + pos);
  math::rotate (ofs.x, ofs.z, &tpos.x, &tpos.z, rotation);
	//if ( (tpos - gWorld->camera).length_squared() > (gWorld->doodaddrawdistance2*model->rad*sc) ) return;
	if (!frustum.intersectsSphere(tpos, model->rad*sc)) return;

  opengl::scoped::matrix_pusher const matrix;

	gl.translatef(pos.x, pos.y, pos.z);
	math::vector_3d vdir(-dir.z, dir.x, dir.y);
	quaternionRotate(vdir, w);
	gl.scalef(sc, -sc, -sc);

	model->draw();
}

void ModelInstance::resetDirection(){
	dir.x = 0;
	//dir.y=0; only reset incline
	dir.z = 0;
}

void ModelInstance::lockUID()
{
	uidLock = true;
}

void ModelInstance::unlockUID()
{
	uidLock = false;
}

bool ModelInstance::hasUIDLock()
{
	return uidLock;
}

bool ModelInstance::isInsideRect(math::vector_3d rect[2])
{
  return misc::rectOverlap(extents, rect);
}

void ModelInstance::recalcExtents()
{
	math::vector_3d min(100000, 100000, 100000);
	math::vector_3d max(-100000, -100000, -100000);
  math::matrix_4x4 rot
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation
                       , { math::degrees (dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (-dir.x)
                         }
                       )
    * math::matrix_4x4 (math::matrix_4x4::scale, sc)
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
		if (bounds[i].x < min.x) min.x = bounds[i].x;
		if (bounds[i].y < min.y) min.y = bounds[i].y;
		if (bounds[i].z < min.z) min.z = bounds[i].z;

		if (bounds[i].x > max.x) max.x = bounds[i].x;
		if (bounds[i].y > max.y) max.y = bounds[i].y;
		if (bounds[i].z > max.z) max.z = bounds[i].z;
	}

	extents[0] = min;
	extents[1] = max;
}
