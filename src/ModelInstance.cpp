#include "ModelInstance.h"

#include "Log.h"
#include "Misc.h" // checkinside
#include "Model.h" // Model, etc.
#include "World.h" // gWorld
#include "Settings.h" // gWorld

Vec3D TransformCoordsForModel(Vec3D pIn)
{
	Vec3D lTemp = pIn;
	lTemp.y = pIn.z;
	lTemp.z = -pIn.y;
	return lTemp;
}

void DrawABox(Vec3D pMin, Vec3D pMax, Vec4D pColor, float pLineWidth)
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
	, nameID(0xFFFFFFFF)
{
}

ModelInstance::ModelInstance(std::string const& filename, MPQFile* f)
	: model (filename)
	, uidLock(false)
	, nameID(SelectionNames.add(this))
{
	float ff[3], temp;
	f->read(ff, 12);
	pos = Vec3D(ff[0], ff[1], ff[2]);
	temp = pos.z;
	pos.z = -pos.y;
	pos.y = temp;
	f->read(&w, 4);
	f->read(ff, 12);
	dir = Vec3D(ff[0], ff[1], ff[2]);
	f->read(&sc, 4);
	f->read(&d1, 4);
	lcol = Vec3D(((d1 & 0xff0000) >> 16) / 255.0f, ((d1 & 0x00ff00) >> 8) / 255.0f, (d1 & 0x0000ff) / 255.0f);
}

ModelInstance::ModelInstance(std::string const& filename, ENTRY_MDDF *d)
	: model (filename)
	, uidLock(false)
	, nameID(0xFFFFFFFF)
{
	d1 = d->uniqueID;
	pos = Vec3D(d->pos[0], d->pos[1], d->pos[2]);
	dir = Vec3D(d->rot[0], d->rot[1], d->rot[2]);
	// scale factor - divide by 1024. blizzard devs must be on crack, why not just use a float?
	sc = d->scale / 1024.0f;
}

void ModelInstance::draw()
{
	/*  float dist = ( pos - gWorld->camera ).length() - model->rad;

	if( dist > 2.0f * gWorld->modeldrawdistance )
	return;
	if( CheckUniques( d1 ) )
	return;*/

	if (!gWorld->frustum.intersectsSphere(pos, model->rad * sc))
		return;

	gl.pushMatrix();

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
		DrawABox(TransformCoordsForModel(model->header.VertexBoxMin), TransformCoordsForModel(model->header.VertexBoxMax), Vec4D(0.5f, 0.5f, 0.5f, 1.0f), 3.0f);
		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (0);
		opengl::texture::enable_texture();
		gl.enable(GL_LIGHTING);
	}
		model->draw();

  bool currentSelection = gWorld->IsSelection(eEntry_Model) && gWorld->GetCurrentSelection()->data.model->d1 == d1;

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

    Vec4D color = model->hidden ? Vec4D(0.0f, 0.0f, 1.0f, 1.0f) : Vec4D(1.0f, 1.0f, 0.0f, 1.0f);

    DrawABox(TransformCoordsForModel(model->header.BoundingBoxMin), TransformCoordsForModel(model->header.BoundingBoxMax), color, 1.0f);

    if (currentSelection)
    {
      DrawABox(TransformCoordsForModel(model->header.VertexBoxMin), TransformCoordsForModel(model->header.VertexBoxMax), Vec4D(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

      gl.color4fv(Vec4D(1.0f, 0.0f, 0.0f, 1.0f));
      gl.begin(GL_LINES);
      gl.vertex3f(0.0f, 0.0f, 0.0f);
      gl.vertex3f(model->header.VertexBoxMax.x + model->header.VertexBoxMax.x / 5.0f, 0.0f, 0.0f);
      gl.end();

      gl.color4fv(Vec4D(0.0f, 1.0f, 0.0f, 1.0f));
      gl.begin(GL_LINES);
      gl.vertex3f(0.0f, 0.0f, 0.0f);
      gl.vertex3f(0.0f, model->header.VertexBoxMax.z + model->header.VertexBoxMax.z / 5.0f, 0.0f);
      gl.end();

      gl.color4fv(Vec4D(0.0f, 0.0f, 1.0f, 1.0f));
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

	gl.popMatrix();
}

//! \todo  Get this drawn on the 2D view.
/*void ModelInstance::drawMapTile()
{
if(CheckUniques(d1))
return;

gl.pushMatrix();

gl.translatef(pos.x/CHUNKSIZE, pos.z/CHUNKSIZE, pos.y);
gl.rotatef(-90.0f, 1, 0, 0);
gl.rotatef(dir.y - 90.0f, 0, 1, 0);
gl.rotatef(-dir.x, 0, 0, 1);
gl.rotatef(dir.z, 1, 0, 0);
gl.scalef(1/CHUNKSIZE,1/CHUNKSIZE,1/CHUNKSIZE);
gl.scalef(sc,sc,sc);

model->draw();

gl.popMatrix();
}*/

void ModelInstance::drawSelect()
{
	/*float dist = ( pos - gWorld->camera ).length() - model->rad;

	if( dist > 2.0f * gWorld->modeldrawdistance )
	return;
	if( CheckUniques( d1 ) )
	return;*/

	if (!gWorld->frustum.intersectsSphere(pos, model->rad * sc))
		return;

	gl.pushMatrix();

	gl.translatef(pos.x, pos.y, pos.z);
	gl.rotatef(dir.y - 90.0f, 0.0f, 1.0f, 0.0f);
	gl.rotatef(-dir.x, 0.0f, 0.0f, 1.0f);
	gl.rotatef(dir.z, 1.0f, 0.0f, 0.0f);
	gl.scalef(sc, sc, sc);

	//if( nameID == 0xFFFFFFFF ) //for what is this line? It cracks model selection after map save! Temporary commenting...
	nameID = SelectionNames.add(this);

	gl.pushName(nameID);

	model->drawSelect();

	gl.popName();

	gl.popMatrix();
}



ModelInstance::~ModelInstance()
{
	if (nameID != 0xFFFFFFFF)
	{
		//Log << "Destroying Selection " << nameID << "\n";
		SelectionNames.del(nameID);
		nameID = 0xFFFFFFFF;
	}
}

void quaternionRotate(const Vec3D& vdir, float w)
{
	gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, math::quaternion (vdir, w)));
}

void ModelInstance::draw2(const Vec3D& ofs, const math::degrees rotation)
{
	Vec3D tpos(ofs + pos);
  math::rotate (ofs.x, ofs.z, &tpos.x, &tpos.z, rotation);
	//if ( (tpos - gWorld->camera).length_squared() > (gWorld->doodaddrawdistance2*model->rad*sc) ) return;
	if (!gWorld->frustum.intersectsSphere(tpos, model->rad*sc)) return;

	gl.pushMatrix();

	gl.translatef(pos.x, pos.y, pos.z);
	Vec3D vdir(-dir.z, dir.x, dir.y);
	quaternionRotate(vdir, w);
	gl.scalef(sc, -sc, -sc);

	model->draw();
	gl.popMatrix();
}

void ModelInstance::draw2Select(const Vec3D& ofs, const math::degrees rotation)
{
	Vec3D tpos(ofs + pos);
  math::rotate(ofs.x, ofs.z, &tpos.x, &tpos.z, rotation);
	if ((tpos - gWorld->camera).length_squared() > ((doodaddrawdistance*doodaddrawdistance)*model->rad*sc)) return;
	if (!gWorld->frustum.intersectsSphere(tpos, model->rad*sc)) return;

	gl.pushMatrix();

	gl.translatef(pos.x, pos.y, pos.z);
	Vec3D vdir(-dir.z, dir.x, dir.y);
	quaternionRotate(vdir, w);
	gl.scalef(sc, -sc, -sc);

	model->drawSelect();
	gl.popMatrix();
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

bool ModelInstance::isInsideTile(Vec3D lTileExtents[2])
{
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

	Vec3D bounds[9];
	Vec3D *ptr = bounds;

	*ptr++ = pos;

	*ptr++ = rot * Vec3D(model->header.BoundingBoxMax.x, 0, -model->header.BoundingBoxMax.y);
	*ptr++ = rot * Vec3D(model->header.BoundingBoxMin.x, 0, -model->header.BoundingBoxMax.y);
	*ptr++ = rot * Vec3D(model->header.BoundingBoxMax.x, 0, -model->header.BoundingBoxMin.y);
	*ptr++ = rot * Vec3D(model->header.BoundingBoxMin.x, 0, -model->header.BoundingBoxMin.y);

	*ptr++ = rot * Vec3D(model->header.VertexBoxMax.x, 0, -model->header.VertexBoxMax.y);
	*ptr++ = rot * Vec3D(model->header.VertexBoxMin.x, 0, -model->header.VertexBoxMax.y);
	*ptr++ = rot * Vec3D(model->header.VertexBoxMax.x, 0, -model->header.VertexBoxMin.y);
	*ptr++ = rot * Vec3D(model->header.VertexBoxMin.x, 0, -model->header.VertexBoxMin.y);

	for (int i = 0; i < 9; ++i)
	{
		if (pointInside(bounds[i], lTileExtents))
		{
			return true;
		}
	}

	return false;
}

bool ModelInstance::isInsideChunk(Vec3D lTileExtents[2])
{
	if (isInsideTile(lTileExtents))
		return true;

	//maybe model > chunk || tile
	recalcExtents();

	for (int i = 0; i < 2; ++i)
		if (pointInside(lTileExtents[i], extents))
			return true;

	return false;
}

void ModelInstance::recalcExtents()
{
	Vec3D min(100000, 100000, 100000);
	Vec3D max(-100000, -100000, -100000);
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

	Vec3D bounds[8 * 2];
	Vec3D *ptr = bounds;

	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMax.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMin.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMin.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMax.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMax.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMax.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMax.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMax.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMin.x, model->header.BoundingBoxMax.y, model->header.BoundingBoxMax.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.BoundingBoxMin.x, model->header.BoundingBoxMin.y, model->header.BoundingBoxMax.z));

	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMax.x, model->header.VertexBoxMax.y, model->header.VertexBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMin.x, model->header.VertexBoxMax.y, model->header.VertexBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMin.x, model->header.VertexBoxMin.y, model->header.VertexBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMax.x, model->header.VertexBoxMin.y, model->header.VertexBoxMin.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMax.x, model->header.VertexBoxMin.y, model->header.VertexBoxMax.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMax.x, model->header.VertexBoxMax.y, model->header.VertexBoxMax.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMin.x, model->header.VertexBoxMax.y, model->header.VertexBoxMax.z));
	*ptr++ = rot * TransformCoordsForModel(Vec3D(model->header.VertexBoxMin.x, model->header.VertexBoxMin.y, model->header.VertexBoxMax.z));


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
