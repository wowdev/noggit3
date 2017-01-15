// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "WMOInstance.h"

#include "Log.h"
#include "MapHeaders.h"
#include "WMO.h" // WMO
#include "World.h" // gWorld
#include "Misc.h" // checkinside
#include <opengl/scoped.hpp>

WMOInstance::WMOInstance(std::string const& filename, MPQFile* _file)
	: wmo(filename)
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
	, pos(math::vector_3d(d->pos[0], d->pos[1], d->pos[2]))
	, dir(math::vector_3d(d->rot[0], d->rot[1], d->rot[2]))
	, mUniqueID(d->uniqueID), mFlags(d->flags)
	, mUnknown(d->unknown), mNameset(d->nameSet)
	, doodadset(d->doodadSet)
	, uidLock(false)
{
	extents[0] = math::vector_3d(d->extents[0][0], d->extents[0][1], d->extents[0][2]);
	extents[1] = math::vector_3d(d->extents[1][0], d->extents[1][1], d->extents[1][2]);
}

WMOInstance::WMOInstance(std::string const& filename)
	: wmo(filename)
	, pos(math::vector_3d(0.0f, 0.0f, 0.0f))
	, dir(math::vector_3d(0.0f, 0.0f, 0.0f))
	, mUniqueID(0)
	, mFlags(0)
	, mUnknown(0)
	, mNameset(0)
	, doodadset(0)
	, uidLock(false)
{
}

void DrawABox(math::vector_3d pMin, math::vector_3d pMax, math::vector_4d pColor, float pLineWidth);

void WMOInstance::draw (Frustum const& frustum)
{
  {
    opengl::scoped::matrix_pusher const matrix;
    gl.translatef(pos.x, pos.y, pos.z);

    const float roty = dir.y - 90.0f;

    gl.rotatef(roty, 0.0f, 1.0f, 0.0f);
    gl.rotatef(-dir.x, 0.0f, 0.0f, 1.0f);
    gl.rotatef(dir.z, 1.0f, 0.0f, 0.0f);

    if (gWorld->IsSelection(eEntry_WMO) && boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->mUniqueID == this->mUniqueID)
      wmo->draw(doodadset, pos, math::degrees (roty), true, true, true, frustum);
    else
      wmo->draw(doodadset, pos, math::degrees (roty), false, false, false, frustum);
  }

  // no need to check showModelFromHiddenList in Environment as it's done beforehand in World::draw()
  if (wmo->hidden || ( gWorld->IsSelection(eEntry_WMO) && boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->mUniqueID == this->mUniqueID))
	{
		gl.disable(GL_LIGHTING);

		gl.disable(GL_COLOR_MATERIAL);
		opengl::texture::set_active_texture (0);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		gl.enable(GL_BLEND);
		gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    math::vector_4d color = wmo->hidden ? math::vector_4d(0.0f, 0.0f, 1.0f, 1.0f) : math::vector_4d(0.0f, 1.0f, 0.0f, 1.0f);
		DrawABox(extents[0], extents[1], color, 1.0f);

		opengl::texture::set_active_texture (1);
		opengl::texture::disable_texture();
		opengl::texture::set_active_texture (0);
		opengl::texture::enable_texture();

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
    * math::matrix_4x4 ( math::matrix_4x4::rotation
                       , { math::degrees (dir.z)
                         , math::degrees (dir.y - 90.0f)
                         , math::degrees (-dir.x)
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
    );

	std::vector<math::vector_3d> bounds (8 * (wmo->nGroups + 1));
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

	for (int i = 0; i < (int)wmo->nGroups; ++i)
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

bool WMOInstance::isInsideRect(math::vector_3d rect[2])
{
  return misc::rectOverlap(extents, rect);
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
	dir = math::vector_3d(0.0f, dir.y, 0.0f);
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
