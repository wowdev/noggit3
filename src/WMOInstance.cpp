#include "WMOInstance.h"

#include "Log.h"
#include "MapHeaders.h"
#include "WMO.h" // WMO
#include "World.h" // gWorld
#include "Misc.h" // checkinside
#include <opengl/scoped.hpp>

WMOInstance::WMOInstance(std::string const& filename, MPQFile* _file)
	: wmo(filename)
	, mSelectionID(SelectionNames.add(this))
	, uidLock(false)
{
	_file->read(&mUniqueID, 4);
	_file->read(&pos, 12);
	_file->read(&dir, 12);
	_file->read(&extents[0], 12);
	_file->read(&extents[1], 12);
	_file->read(&mFlags, 2);
	_file->read(&doodadset, 2);
	_file->read(&mNameset, 2);
	_file->read(&mUnknown, 2);
}

WMOInstance::WMOInstance(std::string const& filename, ENTRY_MODF* d)
	: wmo(filename)
	, pos(Vec3D(d->pos[0], d->pos[1], d->pos[2]))
	, dir(Vec3D(d->rot[0], d->rot[1], d->rot[2]))
	, mUniqueID(d->uniqueID), mFlags(d->flags)
	, mUnknown(d->unknown), mNameset(d->nameSet)
	, doodadset(d->doodadSet)
	, mSelectionID(SelectionNames.add(this))
	, uidLock(false)
{
	extents[0] = Vec3D(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
	extents[1] = Vec3D(d->extents[1][0], d->extents[1][1], d->extents[1][2]);
}

WMOInstance::WMOInstance(std::string const& filename)
	: wmo(filename)
	, pos(Vec3D(0.0f, 0.0f, 0.0f))
	, dir(Vec3D(0.0f, 0.0f, 0.0f))
	, mUniqueID(0)
	, mFlags(0)
	, mUnknown(0)
	, mNameset(0)
	, doodadset(0)
	, mSelectionID(SelectionNames.add(this))
	, uidLock(false)
{
}

WMOInstance::~WMOInstance()
{
  if (mSelectionID != -1)
  {
    SelectionNames.del(mSelectionID);
  }
}

void DrawABox(Vec3D pMin, Vec3D pMax, Vec4D pColor, float pLineWidth);

void WMOInstance::draw (Frustum const& frustum)
{
  {
    opengl::scoped::matrix_pusher const matrix;
    gl.translatef(pos.x, pos.y, pos.z);

    const float roty = dir.y - 90.0f;

    gl.rotatef(roty, 0.0f, 1.0f, 0.0f);
    gl.rotatef(-dir.x, 0.0f, 0.0f, 1.0f);
    gl.rotatef(dir.z, 1.0f, 0.0f, 0.0f);

    if (gWorld->IsSelection(eEntry_WMO) && gWorld->GetCurrentSelection()->data.wmo->mUniqueID == this->mUniqueID)
      wmo->draw(doodadset, pos, math::degrees (roty), true, true, true, frustum);
    else
      wmo->draw(doodadset, pos, math::degrees (roty), false, false, false, frustum);
  }

  // no need to check showModelFromHiddenList in Environment as it's done beforehand in World::draw()
	if (wmo->hidden || ( gWorld->IsSelection(eEntry_WMO) && gWorld->GetCurrentSelection()->data.wmo->mUniqueID == this->mUniqueID))
	{
		gl.disable(GL_LIGHTING);

		gl.disable(GL_COLOR_MATERIAL);
		opengl::texture::set_active_texture (0);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		gl.enable(GL_BLEND);
		gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Vec4D color = wmo->hidden ? Vec4D(0.0f, 0.0f, 1.0f, 1.0f) : Vec4D(0.0f, 1.0f, 0.0f, 1.0f);
		DrawABox(extents[0], extents[1], color, 1.0f);

		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (0);
		opengl::texture::enable_texture();

		gl.enable(GL_LIGHTING);
	}
}

void WMOInstance::recalcExtents()
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
    );

	std::vector<Vec3D> bounds (8 * (wmo->nGroups + 1));
	Vec3D *ptr = bounds.data();
	Vec3D wmoMin(wmo->extents[0].x, wmo->extents[0].z, -wmo->extents[0].y);
	Vec3D wmoMax(wmo->extents[1].x, wmo->extents[1].z, -wmo->extents[1].y);

	*ptr++ = rot * Vec3D(wmoMax.x, wmoMax.y, wmoMin.z);
	*ptr++ = rot * Vec3D(wmoMin.x, wmoMax.y, wmoMin.z);
	*ptr++ = rot * Vec3D(wmoMin.x, wmoMin.y, wmoMin.z);
	*ptr++ = rot * Vec3D(wmoMax.x, wmoMin.y, wmoMin.z);
	*ptr++ = rot * Vec3D(wmoMax.x, wmoMin.y, wmoMax.z);
	*ptr++ = rot * Vec3D(wmoMax.x, wmoMax.y, wmoMax.z);
	*ptr++ = rot * Vec3D(wmoMin.x, wmoMax.y, wmoMax.z);
	*ptr++ = rot * Vec3D(wmoMin.x, wmoMin.y, wmoMax.z);

	for (int i = 0; i < (int)wmo->nGroups; ++i)
	{
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMin.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMin.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMin.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMin.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMax.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMax.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMax.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMax.z);
	}


	for (int i = 0; i < 8 * ((int)wmo->nGroups + 1); ++i)
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

bool WMOInstance::isInsideTile(Vec3D lTileExtents[2])
{
  math::matrix_4x4 rot
    ( math::matrix_4x4 (math::matrix_4x4::translation, pos)
    * math::matrix_4x4 ( math::matrix_4x4::rotation
                       , { math::degrees (dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (-dir.x)
                         }
                       )
    );

	std::vector<Vec3D> bounds (4 * (wmo->nGroups) + 5);
	Vec3D *ptr = bounds.data();
	Vec3D wmoMin(wmo->extents[0].x, wmo->extents[0].z, -wmo->extents[0].y);
	Vec3D wmoMax(wmo->extents[1].x, wmo->extents[1].z, -wmo->extents[1].y);

	*ptr++ = pos;

	*ptr++ = rot * Vec3D(wmoMax.x, 0, wmoMax.z);
	*ptr++ = rot * Vec3D(wmoMax.x, 0, wmoMin.z);
	*ptr++ = rot * Vec3D(wmoMin.x, 0, wmoMax.z);
	*ptr++ = rot * Vec3D(wmoMin.x, 0, wmoMin.z);

	for (int i = 0; i < (int)wmo->nGroups; ++i)
	{
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, 0, wmo->groups[i].BoundingBoxMax.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, 0, wmo->groups[i].BoundingBoxMin.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, 0, wmo->groups[i].BoundingBoxMax.z);
		*ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, 0, wmo->groups[i].BoundingBoxMin.z);
	}


	for (int i = 0; i < 4 * ((int)wmo->nGroups) + 5; ++i)
	{
		if (pointInside(bounds[i], lTileExtents))
		{
			return true;
		}
	}

	return false;
}

bool WMOInstance::isInsideChunk(Vec3D lTileExtents[2])
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

void WMOInstance::drawSelect (Frustum const& frustum)
{
  opengl::scoped::matrix_pusher const matrix;

	gl.translatef(pos.x, pos.y, pos.z);

	const float roty = dir.y - 90.0f;

	gl.rotatef(roty, 0.0f, 1.0f, 0.0f);
	gl.rotatef(-dir.x, 0.0f, 0.0f, 1.0f);
	gl.rotatef(dir.z, 1.0f, 0.0f, 0.0f);

	mSelectionID = SelectionNames.add(this);
	gl.pushName(mSelectionID);

	wmo->drawSelect(doodadset, pos, math::degrees (-roty), frustum);

	gl.popName();
}

/*void WMOInstance::drawPortals()
{
  opengl::scoped::matrix_pusher const matrix;

gl.translatef( pos.x, pos.y, pos.z );

const float roty = dir.y - 90.0f;

gl.rotatef( roty, 0.0f, 1.0f, 0.0f );
gl.rotatef( -dir.x, 0.0f, 0.0f, 1.0f );
gl.rotatef( dir.z, 1.0f, 0.0f, 0.0f );

wmo->drawPortals();
}*/

void WMOInstance::resetDirection()
{
	dir = Vec3D(0.0f, dir.y, 0.0f);
}



void WMOInstance::lockUID()
{
	uidLock = true;
}

void WMOInstance::unlockUID()
{
	uidLock = false;
}

bool WMOInstance::hasUIDLock()
{
	return uidLock;
}
