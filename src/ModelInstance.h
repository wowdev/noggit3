#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include "MapHeaders.h" // ENTRY_MDDF
#include "ModelManager.h"
#include "MPQ.h" // MPQFile
#include "Vec3D.h" // Vec3D

class Model;

class ModelInstance
{
public:
	scoped_model_reference model;
	Vec3D extents[2];

	unsigned int nameID;

	Vec3D pos, dir;

	//! \todo  Get this out and do somehow else.
	unsigned int d1;

	float w, sc;

	Vec3D ldir;
	Vec3D lcol;

	~ModelInstance();
	explicit ModelInstance(std::string const& filename);
	explicit ModelInstance(std::string const& filename, MPQFile* f);
	explicit ModelInstance(std::string const& filename, ENTRY_MDDF *d);

	ModelInstance(ModelInstance const& other) = default;
	ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other)
    : model (std::move (other.model))
    // , extents (other.extents)
    , nameID (other.nameID)
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
    other.nameID = -1;
  }
  ModelInstance& operator= (ModelInstance&& other)
  {
  	std::swap (model, other.model);
    std::swap (extents, other.extents);
    std::swap (nameID, other.nameID);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (d1, other.d1);
    std::swap (w, other.w);
    std::swap (sc, other.sc);
    std::swap (ldir, other.ldir);
    std::swap (lcol, other.lcol);
    std::swap (uidLock, other.uidLock);
    other.nameID = -1;
    return *this;
  }

	void draw();
	void drawMapTile();
	//  void drawHighlight();
	void drawSelect();
	void draw2(const Vec3D& ofs, const float rot);
	void draw2Select(const Vec3D& ofs, const float rot);

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
