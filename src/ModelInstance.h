#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include "MapHeaders.h" // ENTRY_MDDF
#include "ModelManager.h"
#include "MPQ.h" // MPQFile
#include "Vec3D.h" // Vec3D
#include <math/ray.hpp>
#include "Selection.h"

class Frustum;
class Model;

class ModelInstance
{
public:
	scoped_model_reference model;
	Vec3D extents[2];

	Vec3D pos, dir;

	//! \todo  Get this out and do somehow else.
	unsigned int d1;

	float w, sc;

	Vec3D ldir;
	Vec3D lcol;

	explicit ModelInstance(std::string const& filename);
	explicit ModelInstance(std::string const& filename, MPQFile* f);
	explicit ModelInstance(std::string const& filename, ENTRY_MDDF *d);

	ModelInstance(ModelInstance const& other) = default;
	ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other)
    : model (std::move (other.model))
    // , extents (other.extents)
    , pos (other.pos)
    , dir (other.dir)
    , d1 (other.d1)
    , w (other.w)
    , sc (other.sc)
    , ldir (other.ldir)
    , lcol (other.lcol)
    , uidLock (other.uidLock)
  {
    std::swap (extents, other.extents);
  }
  ModelInstance& operator= (ModelInstance&& other)
  {
  	std::swap (model, other.model);
    std::swap (extents, other.extents);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (d1, other.d1);
    std::swap (w, other.w);
    std::swap (sc, other.sc);
    std::swap (ldir, other.ldir);
    std::swap (lcol, other.lcol);
    std::swap (uidLock, other.uidLock);
    return *this;
  }

	void draw (Frustum const&);
	void drawMapTile();
	//  void drawHighlight();
  void intersect (math::ray const&, selection_result*);
	void draw2(const Vec3D& ofs, const math::degrees, Frustum const&);

	void resetDirection();

	bool hasUIDLock();
	void lockUID();
	void unlockUID();

	bool isInsideTile(Vec3D lTileExtents[2]);
	bool isInsideChunk(Vec3D lTileExtents[2]);

	void recalcExtents();

private:
	bool uidLock;
};

#endif // MODELINSTANCE_H
