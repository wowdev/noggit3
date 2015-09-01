#ifndef WMOINSTANCE_H
#define WMOINSTANCE_H

#include <set>
#include <stdint.h>

#include "Vec3D.h" // Vec3D
#include "WMO.h"

class MPQFile;
struct ENTRY_MODF;

class WMOInstance
{
public:
	scoped_wmo_reference wmo;
	Vec3D pos;
	Vec3D  extents[2];
	Vec3D  dir;
	unsigned int mUniqueID;
	uint16_t mFlags;
	uint16_t mUnknown;
	uint16_t mNameset;
	uint16_t doodadset;

public:
	WMOInstance(std::string const& filename, MPQFile* _file);
	WMOInstance(std::string const& filename, ENTRY_MODF* d);
	explicit WMOInstance(std::string const& filename);
	~WMOInstance();

	WMOInstance(WMOInstance& other)
		: wmo(std::move(other.wmo))
		, pos(other.pos)
		// , extents (other.extents)
		, dir(other.dir)
		, mUniqueID(other.mUniqueID)
		, mFlags(other.mFlags)
		, mUnknown(other.mUnknown)
		, mNameset(other.mNameset)
		, doodadset(other.doodadset)
		, uidLock(other.uidLock)
		, mSelectionID(other.mSelectionID)
	{
		//std::copy(std::begin(other.extents), std::end(other.extents), extents);
		std::swap(extents, other.extents);
		other.mSelectionID = -1;
	}

  WMOInstance (WMOInstance&& other)
    : wmo (std::move (other.wmo))
    , pos (other.pos)
    // , extents (other.extents)
    , dir (other.dir)
    , mUniqueID (other.mUniqueID)
    , mFlags (other.mFlags)
    , mUnknown (other.mUnknown)
    , mNameset (other.mNameset)
    , doodadset (other.doodadset)
    , uidLock (other.uidLock)
    , mSelectionID (other.mSelectionID)
  {
	//  std::move(std::begin(other.extents), std::end(other.extents), extents);
    std::swap (extents, other.extents);
    other.mSelectionID = -1;
  }

  WMOInstance& operator= (WMOInstance&& other)
  {
	  std::swap(wmo, other.wmo);
	  std::swap(pos, other.pos);
	  std::swap(extents, other.extents);
	  std::swap(dir, other.dir);
	  std::swap(mUniqueID, other.mUniqueID);
	  std::swap(mFlags, other.mFlags);
	  std::swap(mUnknown, other.mUnknown);
	  std::swap(mNameset, other.mNameset);
	  std::swap(doodadset, other.doodadset);
	  std::swap(uidLock, other.uidLock);
	  std::swap(mSelectionID, other.mSelectionID);
	  other.mSelectionID = -1;
	  return *this;
  }

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
