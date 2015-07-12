#include "MapIndex.h"

#include "MPQ.h"
#include "MapTile.h"
#include "Project.h"
#include "Misc.h"
#include "World.h"
#include "MapChunk.h"

MapIndex::MapIndex(const std::string &pBasename)
	: mHasAGlobalWMO(false)
	, mBigAlpha(false)
	, noadt(false)
	, changed(false)
	, cx(-1)
	, cz(-1)
	, basename(pBasename)
{
	std::stringstream filename;
	filename << "World\\Maps\\" << basename << "\\" << basename << ".wdt";

	MPQFile theFile(filename.str());

	uint32_t fourcc;
	uint32_t size;

	// - MVER ----------------------------------------------

	uint32_t version;

	theFile.read(&fourcc, 4);
	theFile.read(&size, 4);
	theFile.read(&version, 4);

	//! \todo find the correct version of WDT files.
	assert(fourcc == 'MVER' && version == 18);

	// - MHDR ----------------------------------------------

	theFile.read(&fourcc, 4);
	theFile.read(&size, 4);

	assert(fourcc == 'MPHD');

	theFile.read(&mphd, sizeof(MPHD));

	mHasAGlobalWMO = mphd.flags & 1;
	mBigAlpha = (mphd.flags & 4) != 0;

	if (!(mphd.flags & FLAG_SHADING))
	{
		mphd.flags |= FLAG_SHADING;
		changed = true;
	}

	// - MAIN ----------------------------------------------

	theFile.read(&fourcc, 4);
	theFile.seekRelative(4);

	assert(fourcc == 'MAIN');

	/// this is the theory. Sadly, we are also compiling on 64 bit machines with size_t being 8 byte, not 4. Therefore, we can't do the same thing, Blizzard does in its 32bit executable.
	//theFile.read( &(mTiles[0][0]), sizeof( 8 * 64 * 64 ) );

	for (int j = 0; j < 64; ++j)
	{
		for (int i = 0; i < 64; ++i)
		{
			theFile.read(&mTiles[j][i].flags, 4);
			theFile.seekRelative(4);

			std::stringstream filename;
			filename << "World\\Maps\\" << basename << "\\" << basename << "_" << i << "_" << j << ".adt";

			mTiles[j][i].tile = NULL;
			mTiles[j][i].onDisc = MPQFile::existsOnDisk(filename.str());

			if (mTiles[j][i].onDisc && !(mTiles[j][i].flags & 1))
			{
				mTiles[j][i].flags |= 1;
				changed = true;
			}
		}
	}

	if (!theFile.isEof() && mHasAGlobalWMO)
	{
		//! \note We actually don't load WMO only worlds, so we just stop reading here, k?
		//! \bug MODF reads wrong. The assertion fails every time. Somehow, it keeps being MWMO. Or are there two blocks?
		//! \nofuckingbug  on eof read returns just without doing sth to the var and some wdts have a MWMO without having a MODF so only checking for eof above is not enough

		mHasAGlobalWMO = false;

		// - MWMO ----------------------------------------------

		theFile.read(&fourcc, 4);
		theFile.read(&size, 4);

		assert(fourcc == 'MWMO');

		globalWMOName = std::string(theFile.getPointer(), size);
		theFile.seekRelative(size);

		// - MODF ----------------------------------------------

		theFile.read(&fourcc, 4);
		theFile.read(&size, 4);

		assert(fourcc == 'MODF');

		theFile.read(&wmoEntry, sizeof(ENTRY_MODF));
	}

	// -----------------------------------------------------

	theFile.close();
}

MapIndex::~MapIndex()
{
	for (int j = 0; j < 64; ++j)
	{
		for (int i = 0; i < 64; ++i)
		{
			if (tileLoaded(j, i))
			{
				delete mTiles[j][i].tile;
				mTiles[j][i].tile = NULL;
			}
		}
	}
}

void MapIndex::save()
{
	std::stringstream filename;
	filename << "World\\Maps\\" << basename << "\\" << basename << ".wdt";

	//Log << "Saving WDT \"" << filename << "\"." << std::endl;

	sExtendableArray wdtFile = sExtendableArray();
	int curPos = 0;

	// MVER
	//  {
	wdtFile.Extend(8 + 0x4);
	SetChunkHeader(wdtFile, curPos, 'MVER', 4);

	// MVER data
	*(wdtFile.GetPointer<int>(8)) = 18;

	curPos += 8 + 0x4;
	//  }

	// MPHD
	//  {
	wdtFile.Extend(8);
	SetChunkHeader(wdtFile, curPos, 'MPHD', sizeof(MPHD));
	curPos += 8;

	wdtFile.Insert(curPos, sizeof(MPHD), (char*)&mphd);
	curPos += sizeof(MPHD);
	//  }

	// MAIN
	//  {
	wdtFile.Extend(8);
	SetChunkHeader(wdtFile, curPos, 'MAIN', 64 * 64 * 8);
	curPos += 8;

	for (int j = 0; j < 64; ++j)
	{
		for (int i = 0; i < 64; ++i)
		{
			wdtFile.Insert(curPos, 4, (char*)&mTiles[j][i].flags);
			wdtFile.Extend(4);
			curPos += 8;
		}
	}
	//  }

	if (mHasAGlobalWMO)
	{
		// MWMO
		//  {
		wdtFile.Extend(8);
		SetChunkHeader(wdtFile, curPos, 'MWMO', globalWMOName.size());
		curPos += 8;

		wdtFile.Insert(curPos, globalWMOName.size(), globalWMOName.data());
		curPos += globalWMOName.size();
		//  }

		// MODF
		//  {
		wdtFile.Extend(8);
		SetChunkHeader(wdtFile, curPos, 'MODF', sizeof(ENTRY_MODF));
		curPos += 8;

		wdtFile.Insert(curPos, sizeof(ENTRY_MODF), (char*)&wmoEntry);
		curPos += sizeof(ENTRY_MODF);
		//  }
	}

	MPQFile f(filename.str());
	f.setBuffer(wdtFile.GetPointer<char>(), wdtFile.mSize);
	f.SaveFile();
	f.close();

	changed = false;
}

void MapIndex::enterTile(int x, int z)
{
	if (!hasTile(z, x))
	{
		noadt = true;
		return;
	}

	noadt = false;

	cx = x;
	cz = z;
	for (int i = std::max(cz - 2, 0); i < std::min(cz + 2, 64); ++i)
	{
		for (int j = std::max(cx - 2, 0); j < std::min(cx + 2, 64); ++j)
		{
			mTiles[i][j].tile = loadTile(i, j);
		}
	}

	if (autoheight && tileLoaded(cz, cx)) //ZX STEFF HERE SWAP!
	{
		float maxHeight = mTiles[cz][cx].tile->getMaxHeight();
		maxHeight = std::max(maxHeight, 0.0f);
		gWorld->camera.y = maxHeight + 50.0f;

		autoheight = false;
	}
}

void MapIndex::setChanged(float x, float z)
{
	// change the changed flag of the map tile
	int row = misc::FtoIround((x - (TILESIZE / 2)) / TILESIZE);
	int column = misc::FtoIround((z - (TILESIZE / 2)) / TILESIZE);

	if (row >= 0 && row <= 64 && column >= 0 && column <= 64)
		setChanged(column, row);
}

void MapIndex::setChanged(int x, int z)
{
	// change the changed flag of the map tile
	if (hasTile(x, z))
	{
		if (!mTiles[x][z].tile)
			loadTile(x, z);

		if (mTiles[x][z].tile->changed == 1)
			return;

		mTiles[x][z].tile->changed = 1;
	}

	for (int posaddx = -1; posaddx < 2; posaddx++)
	{
		for (int posaddz = -1; posaddz < 2; posaddz++)
		{
			if (!hasTile(x + posaddx, z + posaddz))
				continue;

			if (!mTiles[x + posaddx][z + posaddz].tile)
				loadTile(x + posaddx, z + posaddz);

			if (mTiles[x + posaddx][z + posaddz].tile->changed == 1)
				continue;

			mTiles[x + posaddx][z + posaddz].tile->changed = 2;
		}
	}

}

void MapIndex::unsetChanged(int x, int z)
{
	// change the changed flag of the map tile
	if (mTiles[x][z].tile)
		mTiles[x][z].tile->changed = 0;
}

int MapIndex::getChanged(int x, int z)
{
	if (mTiles[x][z].tile) // why do we need to save tile with changed=2? What "2" means? its adts which have models with new adts, and who ever added this here broke everything, thanks
		return mTiles[x][z].tile->changed;
	else
		return 0;
}

void MapIndex::setFlag(bool to, float x, float z)
{
	// set the inpass flag to selected chunk
	this->setChanged(x, z);
	const int newX = (const int)(x / TILESIZE);
	const int newZ = (const int)(z / TILESIZE);

	for (int j = newZ - 1; j < newZ + 1; ++j)
	{
		for (int i = newX - 1; i < newX + 1; ++i)
		{
			if (tileLoaded(j, i))
			{
				for (int ty = 0; ty < 16; ++ty)
				{
					for (int tx = 0; tx < 16; ++tx)
					{
						MapChunk* chunk = mTiles[j][i].tile->getChunk(ty, tx);
						if (chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z)
						{
							chunk->setFlag(to);
						}
					}
				}
			}
		}
	}
}

void MapIndex::setWater(bool to, float x, float z)
{
	// set the inpass flag to selected chunk
	this->setChanged(x, z);
	const int newX = (const int)(x / TILESIZE);
	const int newZ = (const int)(z / TILESIZE);

	for (int j = newZ - 1; j < newZ + 1; ++j)
	{
		for (int i = newX - 1; i < newX + 1; ++i)
		{
			if (tileLoaded(j, i))
			{
				for (int ty = 0; ty < 16; ++ty)
				{
					for (int tx = 0; tx < 16; ++tx)
					{
						MapChunk* chunk = mTiles[j][i].tile->getChunk(ty, tx);
						if (chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z)
						{
							chunk->SetWater(to);
						}
					}
				}
			}
		}
	}
}

MapTile* MapIndex::loadTile(int z, int x)
{
	if (!hasTile(z, x))
	{
		return NULL;
	}

	if (tileLoaded(z, x))
	{
		return mTiles[z][x].tile;
	}

	std::stringstream filename;
	filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

	if (!MPQFile::exists(filename.str()))
	{
		LogError << "The requested tile \"" << filename.str() << "\" does not exist! Oo" << std::endl;
		return NULL;
	}

	if (mTiles[z][x].tile) //just to sure
	{
		delete mTiles[z][x].tile;
		mTiles[z][x].tile = NULL;
	}

	mTiles[z][x].tile = new MapTile(x, z, filename.str(), mBigAlpha);// XZ STEFF Swap MapTile( z, x, file
	return mTiles[z][x].tile;
}

void MapIndex::reloadTile(int x, int z)
{
	if (tileLoaded(z, x))
	{
		delete mTiles[z][x].tile;
		mTiles[z][x].tile = NULL;

		std::stringstream filename;
		filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

		mTiles[z][x].tile = new MapTile(x, z, filename.str(), mBigAlpha);
		enterTile(cx, cz);
	}
}

void MapIndex::markOnDisc(int x, int z, bool mto)
{
	mTiles[z][x].onDisc = mto;
}

bool MapIndex::isTileExternal(int x, int z)
{
	// is onDisc
	return mTiles[z][x].onDisc;
}

void MapIndex::saveTile(int x, int z)
{
	// save goven tile
	if (tileLoaded(z, x))
	{
		mTiles[z][x].tile->saveTile();
	}
}

void MapIndex::saveChanged()
{
	if (changed)
		save();

	// Now save all marked as 1 and 2 because UIDs now fits.
	for (int j = 0; j < 64; ++j)
	{
		for (int i = 0; i < 64; ++i)
		{
			if (!tileLoaded(j, i)) continue;
			if (!getChanged(j, i)) continue;

			mTiles[j][i].tile->saveTile();
			unsetChanged(j, i);
		}
	}

	std::map<int, WMOInstance> wmoTemp(gWorld->mWMOInstances);
	gWorld->mWMOInstances.clear();

	for (std::map<int, WMOInstance>::iterator it = wmoTemp.begin(); it != wmoTemp.end(); ++it)
	{
		gWorld->mWMOInstances[it->second.mUniqueID] = it->second;
		it->second.unlockUID();
	}

	std::map<int, ModelInstance> modelTemp(gWorld->mModelInstances);
	gWorld->mModelInstances.clear();

	for (std::map<int, ModelInstance>::iterator it = modelTemp.begin(); it != modelTemp.end(); ++it)
	{
		gWorld->mModelInstances[it->second.d1] = it->second;
		it->second.unlockUID();
	}

	wmoTemp.clear();
	modelTemp.clear();
}

bool MapIndex::hasAGlobalWMO()
{
	return mHasAGlobalWMO;
}

bool MapIndex::oktile(int z, int x)
{
	return !(z < 0 || x < 0 || z > 64 || x > 64);
}

bool MapIndex::hasTile(int pZ, int pX)
{
	return oktile(pZ, pX) && (mTiles[pZ][pX].flags & 1);
}

bool MapIndex::tileLoaded(int z, int x)
{
	return hasTile(z, x) && mTiles[z][x].tile;
}

bool MapIndex::hasAdt()
{
	return noadt;
}

void MapIndex::setAdt(bool value)
{
	noadt = value;
}

MapTile* MapIndex::getTile(size_t z, size_t x)
{
	return mTiles[z][x].tile;
}

uint32_t MapIndex::getFlag(size_t z, size_t x)
{
	return mTiles[z][x].flags;
}
