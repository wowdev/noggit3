#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include "Vec3D.h" // Vec3D
#include "MPQ.h" // MPQFile
#include "MapHeaders.h" // ENTRY_MDDF

class Model;

class ModelInstance
{
public:
	Model *model;
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
