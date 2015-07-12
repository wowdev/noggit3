#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>
#include <stdint.h>

#include "Vec3D.h" // Vec3D

class WMO;
class MPQFile;
struct ENTRY_MODF;

class WMOInstance
{
public:
	WMO* wmo;
	Vec3D pos;
	Vec3D  extents[2];
	Vec3D  dir;
	unsigned int mUniqueID;
	uint16_t mFlags;
	uint16_t mUnknown;
	uint16_t mNameset;
	uint16_t doodadset;

public:
	WMOInstance();
	WMOInstance(WMO* _wmo, MPQFile* _file);
	WMOInstance(WMO* _wmo, ENTRY_MODF* d);
	explicit WMOInstance(WMO* _wmo);
	~WMOInstance();

	void draw();
	void drawSelect();

	void recalcExtents();
	void resetDirection();

	bool isInsideTile(Vec3D lTileExtents[2]);
	bool isInsideChunk(Vec3D lTileExtents[2]);

	bool hasUIDLock();
	void lockUID();
	void unlockUID();

private:
	bool uidLock;
	unsigned int mSelectionID;
};


#endif // WMOINSTANCE_H
