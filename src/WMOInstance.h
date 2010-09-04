#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>

#include "vec3d.h" // Vec3D
#include "mpq.h" // MPQFile
#include "mapheaders.h" // ENTRY_MODF

class WMO;

class WMOInstance {
	static std::set<int> ids;
public:
	WMO *wmo;
	Vec3D pos;
	Vec3D	extents[2];
	Vec3D	dir;
	int id;
	short mFlags, mUnknown, mNameset, doodadset;
	unsigned int nameID;
	unsigned int wmoID;

	WMOInstance(WMO *wmo, MPQFile &f);
	WMOInstance(WMO *wmo, ENTRY_MODF *d);
	WMOInstance(WMO *wmo);
	void draw();
	void drawSelect();
	//void drawPortals();
	
	void resetPosition(); 
	void resetDirection(); 

	static void reset();
	~WMOInstance();
};


#endif // WMOINSTANCE_H
