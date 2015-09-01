#ifndef MAPINDEX_H
#define MAPINDEX_H

#include <string>
#include <stdint.h>
#include <sstream>
#include <fstream>

#include "MapHeaders.h"

class MapTile;

/*!
\brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
*/
class MapTileEntry
{
private:
	uint32_t flags;
	MapTile* tile;
	bool onDisc;


	MapTileEntry() : flags(0), tile(NULL) {}

	friend class MapIndex;
};

class MapIndex
{
public:
	MapIndex(const std::string& pBasename);
	~MapIndex();

	void enterTile(int x, int z);
	MapTile *loadTile(int x, int z);

	void setChanged(float x, float z);
	void setChanged(int x, int z);

	void unsetChanged(int x, int z);
	void setFlag(bool to, float x, float z);
	void setWater(bool to, float x, float z);
	int getChanged(int x, int z);

	void saveTile(int x, int z);
	void saveChanged();
	void reloadTile(int x, int z);					
	void unloadTiles(int x, int z);					// unloads all tiles more then x adts away fr0m given
	void unloadTile(int x, int z);					// unload given tile
	void markOnDisc(int x, int z, bool mto);
	bool isTileExternal(int x, int z);

	bool hasAGlobalWMO();
	bool oktile(int z, int x);
	bool hasTile(int pZ, int pX);
	bool tileLoaded(int z, int x);

	bool hasAdt();
	void setAdt(bool value);

	void save();

	MapTile* getTile(size_t z, size_t x);
	uint32_t getFlag(size_t z, size_t x);

private:
	const std::string basename;
	std::string globalWMOName;

	int unloadTimeDelay;

	// Is the WDT telling us to use a different alphamap structure.
	bool mBigAlpha;
	bool mHasAGlobalWMO;
	bool noadt;
	bool changed;

	bool autoheight;

	int cx;
	int cz;

	ENTRY_MODF wmoEntry;
	MPHD mphd;

	// Holding all MapTiles there can be in a World.
	MapTileEntry mTiles[64][64];
};

#endif //MAPINDEX_H
