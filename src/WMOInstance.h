#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>

#include "vec3d.h" // Vec3D
#include "mpq.h" // MPQFile
#include "mapheaders.h" // ENTRY_MODF

class WMO;

class WMOInstance
{
public:
	WMO* wmo;
	Vec3D pos;
	Vec3D	extents[2];
	Vec3D	dir;
	int mUniqueID;
	short mFlags;
	short mUnknown;
	short mNameset;
	short doodadset;
	
private:
	unsigned int mSelectionID;
	
public:
	WMOInstance( WMO* _wmo, MPQFile& _file );
	WMOInstance( WMO* _wmo, ENTRY_MODF* d );
	WMOInstance( WMO* _wmo );
	~WMOInstance();

	void draw() const;
	void drawSelect() const;

	void resetDirection();
};


#endif // WMOINSTANCE_H
