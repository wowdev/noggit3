// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/Project.h>
#include <noggit/World.h>
#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>
#endif
#include <noggit/map_index.hpp>
#include <noggit/uid_storage.hpp>

#include <boost/range/adaptor/map.hpp>

#include <forward_list>

MapIndex::MapIndex(const std::string &pBasename)
  : basename(pBasename)
  , mBigAlpha(false)
  , mHasAGlobalWMO(false)
  , noadt(false)
  , changed(false)
  , cx(-1)
  , cz(-1)
  , highestGUID(0)
  , highestGUIDDB(0)
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

      mTiles[j][i].tile = nullptr;
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

void MapIndex::saveall()
{
  for (MapTile* tile : loaded_tiles())
  {
    tile->saveTile();
    tile->changed = 0;
  }
}

void MapIndex::savecurrent()
{
  gWorld->mapIndex.saveTile(tile_index(gWorld->camera));
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
  f.setBuffer(wdtFile.data);
  f.SaveFile();
  f.close();

  changed = false;
}

void MapIndex::enterTile(const tile_index& tile)
{
  if (!hasTile(tile))
  {
    noadt = true;
    return;
  }

  noadt = false;
  cx = tile.x;
  cz = tile.z;

  for (int pz = std::max(cz - 1, 0); pz < std::min(cz + 2, 63); ++pz)
  {
    for (int px = std::max(cx - 1, 0); px < std::min(cx + 2, 63); ++px)
    {
      loadTile(tile_index(px, pz));
    }
  }

  if (autoheight && tileLoaded(tile))
  {
    float maxHeight = mTiles[cz][cx].tile->getMaxHeight();
    maxHeight = std::max(maxHeight, 0.0f);
    gWorld->camera.y = maxHeight + 50.0f;

    autoheight = false;
  }
}

void MapIndex::setChanged(const tile_index& tile)
{
  MapTile* mTile = loadTile(tile);

  if (!!mTile)
  {
    mTile->changed = 1;
  }
}

void MapIndex::setChanged(MapTile* tile)
{
  setChanged(tile->index);
}

void MapIndex::unsetChanged(const tile_index& tile)
{
  // change the changed flag of the map tile
  if (mTiles[tile.z][tile.x].tile)
  {
    mTiles[tile.z][tile.x].tile->changed = 0;
  }
}

int MapIndex::getChanged(const tile_index& tile)
{
  // Changed 2 are adts around the changed one that have 1 in changed.
  // You must save them also IF you do any UID recalculation on changed 1 adts.
  // Because the new UIDs MUST also get saved in surrounding adts to ahve no model duplucation.
  // So to avoid unnneeded save you can also skip changed 2 adts IF no models get added or moved around.
  // This would be stepp to IF uid workes. Steff
  if (mTiles[tile.z][tile.x].tile) // why do we need to save tile with changed=2? What "2" means? its adts which have models with new adts, and who ever added this here broke everything, thanks
    return mTiles[tile.z][tile.x].tile->changed;
  else
    return 0;
}

void MapIndex::setFlag(bool to, math::vector_3d const& pos, uint32_t flag)
{
  tile_index tile(pos);

  if (tileLoaded(tile))
  {
    setChanged(tile);

    int cx = (pos.x - tile.x * TILESIZE) / CHUNKSIZE;
    int cz = (pos.z - tile.z * TILESIZE) / CHUNKSIZE;

    getTile(tile)->getChunk(cx, cz)->setFlag(to, flag);
  }
}

MapTile* MapIndex::loadTile(const tile_index& tile)
{
  if (!hasTile(tile))
  {
    return nullptr;
  }

  if (tileLoaded(tile))
  {
    return mTiles[tile.z][tile.x].tile.get();
  }

  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << "_" << tile.x << "_" << tile.z << ".adt";

  if (!MPQFile::exists(filename.str()))
  {
    LogError << "The requested tile \"" << filename.str() << "\" does not exist! Oo" << std::endl;
    return nullptr;
  }

  mTiles[tile.z][tile.x].tile = std::make_unique<MapTile> (tile.x, tile.z, filename.str(), mBigAlpha);

  return mTiles[tile.z][tile.x].tile.get();
}

void MapIndex::reloadTile(const tile_index& tile)
{
  if (tileLoaded(tile))
  {
    mTiles[tile.z][tile.x].tile = nullptr;

    std::stringstream filename;
    filename << "World\\Maps\\" << basename << "\\" << basename << "_" << tile.x << "_" << tile.z << ".adt";

    mTiles[tile.z][tile.x].tile = std::make_unique<MapTile> (tile.x, tile.z, filename.str(), mBigAlpha);
    enterTile(tile_index(cx, cz));
  }
}

void MapIndex::unloadTiles(const tile_index& tile)
{
  if ( ((clock() / CLOCKS_PER_SEC) - this->lastUnloadTime) > 5) // only unload every 5 seconds
  {
    int unloadBoundery = 6; // means noggit hold always plus X adts in all direction in ram - perhaps move this into settings file?
    for (int pz = 0; pz < 64; ++pz)
    {
      for (int px = 0; px < 64; ++px)
      {
        if (std::abs(px - tile.x) > unloadBoundery || std::abs(pz - tile.z) > unloadBoundery)
        {
          tile_index id(px, pz);

          //Only unload adts not marked to save
          if (getChanged(id) == 0)
          {
            unloadTile(id);
          }
        }
      }
    }
    this->lastUnloadTime = clock() / CLOCKS_PER_SEC;
  }
}

void MapIndex::unloadTile(const tile_index& tile)
{
  // unloads a tile with givn cords
  if (tileLoaded(tile))
  {
    mTiles[tile.z][tile.x].tile = nullptr;
    Log << "Unload Tile " << tile.x << "-" << tile.z << "\n";
  }
}

void MapIndex::markOnDisc(const tile_index& tile, bool mto)
{
  mTiles[tile.z][tile.x].onDisc = mto;
}

bool MapIndex::isTileExternal(const tile_index& tile)
{
  // is onDisc
  return mTiles[tile.z][tile.x].onDisc;
}

void MapIndex::saveTile(const tile_index& tile)
{
	// save given tile
	if (tileLoaded(tile))
	{
    saveMaxUID();
		mTiles[tile.z][tile.x].tile->saveTile();
	}
}

void MapIndex::saveChanged()
{
  if (changed)
    save();

  saveMaxUID();

  for (MapTile* tile : loaded_tiles())
  {
    if (tile->changed)
    {
      tile->saveTile();
      tile->changed = 0;
    }
  }
}

bool MapIndex::hasAGlobalWMO()
{
  return mHasAGlobalWMO;
}


bool MapIndex::hasTile(const tile_index& tile) const
{
  return (mTiles[tile.z][tile.x].flags & 1);
}

bool MapIndex::hasTile(int tileX, int tileZ) const
{
  return (mTiles[tileZ][tileX].flags & 1);
}

bool MapIndex::tileLoaded(const tile_index& tile) const
{
  return hasTile(tile) && mTiles[tile.z][tile.x].tile;
}

bool MapIndex::tileLoaded(int tileX, int tileZ) const
{
  return hasTile(tileX, tileZ) && mTiles[tileZ][tileX].tile;
}

bool MapIndex::hasAdt()
{
  return noadt;
}

void MapIndex::setAdt(bool value)
{
  noadt = value;
}

MapTile* MapIndex::getTile(const tile_index& tile) const
{
  return mTiles[tile.z][tile.x].tile.get();
}

MapTile* MapIndex::getTileAbove(MapTile* tile) const
{
  if (tile->index.z == 0 || !tileLoaded(tile->index.x, tile->index.z - 1))
  {
    return nullptr;
  }

  return mTiles[tile->index.z - 1][tile->index.x].tile.get();
}

MapTile* MapIndex::getTileLeft(MapTile* tile) const
{
  if (tile->index.x == 0 || !tileLoaded(tile->index.x - 1, tile->index.z))
  {
    return nullptr;
  }

  return mTiles[tile->index.z][tile->index.x - 1].tile.get();
}

uint32_t MapIndex::getFlag(const tile_index& tile) const
{
  return mTiles[tile.z][tile.x].flags;
}

void MapIndex::convert_alphamap(bool to_big_alpha)
{
  mBigAlpha = to_big_alpha;
  if (to_big_alpha)
  {
    mphd.flags |= 4;
  }
  else
  {
    mphd.flags &= 0xFFFFFFFB;
  }
}


uint32_t MapIndex::getHighestGUIDFromFile(const std::string& pFilename) const
{
	uint32_t highGUID = 0;

    MPQFile theFile(pFilename);
    if (theFile.isEof())
    {
      return highGUID;
    }

    uint32_t fourcc;
    uint32_t size;

    MHDR Header;

    // - MVER ----------------------------------------------

    uint32_t version;

    theFile.read(&fourcc, 4);
    theFile.seekRelative(4);
    theFile.read(&version, 4);

    assert(fourcc == 'MVER' && version == 18);

    // - MHDR ----------------------------------------------

    theFile.read(&fourcc, 4);
    theFile.seekRelative(4);

    assert(fourcc == 'MHDR');

    theFile.read(&Header, sizeof(MHDR));

    // - MDDF ----------------------------------------------

    theFile.seek(Header.mddf + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MDDF');

    ENTRY_MDDF const* mddf_ptr = reinterpret_cast<ENTRY_MDDF const*>(theFile.getPointer());
    for (unsigned int i = 0; i < size / sizeof(ENTRY_MDDF); ++i)
    {
        highGUID = std::max(highGUID, mddf_ptr[i].uniqueID);
    }

    // - MODF ----------------------------------------------

    theFile.seek(Header.modf + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MODF');

    ENTRY_MODF const* modf_ptr = reinterpret_cast<ENTRY_MODF const*>(theFile.getPointer());
    for (unsigned int i = 0; i < size / sizeof(ENTRY_MODF); ++i)
    {
        highGUID = std::max(highGUID, modf_ptr[i].uniqueID);
    }
    theFile.close();

    return highGUID;
}

#ifdef USE_MYSQL_UID_STORAGE
uint32_t MapIndex::getHighestGUIDFromDB() const
{
	return mysql::getGUIDFromDB (*Settings::getInstance()->mysql, gWorld->mMapId);
}

uint32_t MapIndex::newGUIDDB()
{
  highestGUIDDB = std::max(highestGUIDDB, getHighestGUIDFromDB());
  highGUIDDB = std::max(highestGUID, highestGUIDDB);
  highGUIDDB = ++highestGUIDDB;
  highestGUID = highGUIDDB; // update local max uid too
  mysql::updateUIDinDB(*Settings::getInstance()->mysql, gWorld->mMapId, highGUIDDB);  // it's neccesary to update the uid in database after every place, other then in the file uid storage system, because of cloudworking
  return highGUIDDB;
}
#endif

uint32_t MapIndex::newGUID()
{
#ifdef USE_MYSQL_UID_STORAGE
  if (Settings::getInstance()->mysql) {
    return newGUIDDB();
  }
  else {
  return ++highestGUID;
  }
#else
  return ++highestGUID;
#endif
}

inline bool floatEqual(float const& a, float const& b)
{
  return std::abs(a - b) < 0.0001f;
}

void MapIndex::fixUIDs()
{
  // pre-cond: mTiles[z][x].flags are set

  std::forward_list<ModelInstance> models;
  std::forward_list<WMOInstance> wmos;

  for (int z = 0; z < 64; ++z)
  {
    for (int x = 0; x < 64; ++x)
    {
      if (!(mTiles[z][x].flags & 1))
      {
        continue;
      }

      std::stringstream filename;
      filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";
      MPQFile file(filename.str());

      if (file.isEof())
      {
        continue;
      }

      math::vector_3d tileExtents[2];
      tileExtents[0] = { x*TILESIZE, 0, z*TILESIZE };
      tileExtents[1] = { (x+1)*TILESIZE, 0, (z+1)*TILESIZE };

      std::forward_list<ENTRY_MDDF> modelEntries;
      std::forward_list<ENTRY_MODF> wmoEntries;
      std::vector<std::string> modelFilenames;
      std::vector<std::string> wmoFilenames;

      uint32_t fourcc;
      uint32_t size;

      MHDR Header;

      // - MVER ----------------------------------------------
      uint32_t version;
      file.read(&fourcc, 4);
      file.seekRelative(4);
      file.read(&version, 4);
      assert(fourcc == 'MVER' && version == 18);

      // - MHDR ----------------------------------------------
      file.read(&fourcc, 4);
      file.seekRelative(4);
      assert(fourcc == 'MHDR');
      file.read(&Header, sizeof(MHDR));

      // - MDDF ----------------------------------------------
      file.seek(Header.mddf + 0x14);
      file.read(&fourcc, 4);
      file.read(&size, 4);
      assert(fourcc == 'MDDF');

      ENTRY_MDDF const* mddf_ptr = reinterpret_cast<ENTRY_MDDF const*>(file.getPointer());

      for (unsigned int i = 0; i < size / sizeof(ENTRY_MDDF); ++i)
      {
        bool add = true;
        ENTRY_MDDF const& mddf = mddf_ptr[i];

        if (!pointInside({ mddf.pos[0], 0, mddf.pos[2] }, tileExtents))
        {
          continue;
        }

        // check for duplicates
        for (ENTRY_MDDF& entry : modelEntries)
        {
          if ( mddf.nameID == entry.nameID
            && floatEqual(mddf.pos[0], entry.pos[0])
            && floatEqual(mddf.pos[1], entry.pos[1])
            && floatEqual(mddf.pos[2], entry.pos[2])
            && floatEqual(mddf.rot[0], entry.rot[0])
            && floatEqual(mddf.rot[1], entry.rot[1])
            && floatEqual(mddf.rot[2], entry.rot[2])
            && mddf.scale == entry.scale
            )
          {
            add = false;
            break;
          }
        }

        if (add)
        {
          modelEntries.emplace_front(mddf);
        }
      }

      // - MODF ----------------------------------------------
      file.seek(Header.modf + 0x14);
      file.read(&fourcc, 4);
      file.read(&size, 4);
      assert(fourcc == 'MODF');

      ENTRY_MODF const* modf_ptr = reinterpret_cast<ENTRY_MODF const*>(file.getPointer());

      for (unsigned int i = 0; i < size / sizeof(ENTRY_MODF); ++i)
      {
        bool add = true;
        ENTRY_MODF const& modf = modf_ptr[i];

        if (!pointInside({ modf.pos[0], 0, modf.pos[2] }, tileExtents))
        {
          continue;
        }

        // check for duplicates
        for (ENTRY_MODF& entry : wmoEntries)
        {
          if (modf.nameID == entry.nameID
            && floatEqual(modf.pos[0], entry.pos[0])
            && floatEqual(modf.pos[1], entry.pos[1])
            && floatEqual(modf.pos[2], entry.pos[2])
            && floatEqual(modf.rot[0], entry.rot[0])
            && floatEqual(modf.rot[1], entry.rot[1])
            && floatEqual(modf.rot[2], entry.rot[2])
            )
          {
            add = false;
            break;
          }
        }

        if (add)
        {
          wmoEntries.emplace_front(modf);
        }
      }

      // - MMDX ----------------------------------------------
      file.seek(Header.mmdx + 0x14);
      file.read(&fourcc, 4);
      file.read(&size, 4);
      assert(fourcc == 'MMDX');

      {
        char const* lCurPos = reinterpret_cast<char const*>(file.getPointer());
        char const* lEnd = lCurPos + size;

        while (lCurPos < lEnd)
        {
          modelFilenames.push_back(std::string(lCurPos));
          lCurPos += strlen(lCurPos) + 1;
        }
      }

      // - MWMO ----------------------------------------------
      file.seek(Header.mwmo + 0x14);
      file.read(&fourcc, 4);
      file.read(&size, 4);
      assert(fourcc == 'MWMO');

      {
        char const* lCurPos = reinterpret_cast<char const*>(file.getPointer());
        char const* lEnd = lCurPos + size;

        while (lCurPos < lEnd)
        {
          wmoFilenames.push_back(std::string(lCurPos));
          lCurPos += strlen(lCurPos) + 1;
        }
      }

      file.close();

      for (ENTRY_MDDF& entry : modelEntries)
      {
        models.emplace_front(modelFilenames[entry.nameID], &entry);
      }
      for (ENTRY_MODF& entry : wmoEntries)
      {
        wmos.emplace_front(wmoFilenames[entry.nameID], &entry);
      }
    }
  }

  // set all uids
  // for each tile save the m2/wmo present inside
  uint32_t uid{ 0 };
  std::map<std::size_t, std::map<std::size_t, std::forward_list<ModelInstance*>>> modelPerTile;
  std::map<std::size_t, std::map<std::size_t, std::forward_list<WMOInstance*>>> wmoPerTile;

  for (ModelInstance& instance : models)
  {
    instance.d1 = uid++;

    // to avoid going outside of bound
    std::size_t sx = std::max((std::size_t)(instance.extents[0].x / TILESIZE), (std::size_t)0);
    std::size_t sz = std::max((std::size_t)(instance.extents[0].z / TILESIZE), (std::size_t)0);
    std::size_t ex = std::min((std::size_t)(instance.extents[1].x / TILESIZE), (std::size_t)63);
    std::size_t ez = std::min((std::size_t)(instance.extents[1].z / TILESIZE), (std::size_t)63);


    for (std::size_t z = sz; z <= ez; ++z)
    {
      for (std::size_t x = sx; x <= ex; ++x)
      {
        modelPerTile[z][x].push_front(&instance);
      }
    }
  }

  for (WMOInstance& instance : wmos)
  {
    instance.mUniqueID = uid++;

    // to avoid going outside of bound
    std::size_t sx = std::max((std::size_t)(instance.extents[0].x / TILESIZE), (std::size_t)0);
    std::size_t sz = std::max((std::size_t)(instance.extents[0].z / TILESIZE), (std::size_t)0);
    std::size_t ex = std::min((std::size_t)(instance.extents[1].x / TILESIZE), (std::size_t)63);
    std::size_t ez = std::min((std::size_t)(instance.extents[1].z / TILESIZE), (std::size_t)63);

    for (std::size_t z = sz; z <= ez; ++z)
    {
      for (std::size_t x = sx; x <= ex; ++x)
      {
        wmoPerTile[z][x].push_front(&instance);
      }
    }
  }

  // save the current highest guid
  highestGUID = uid - 1;

  // load each tile without the models and
  // save them with the models from modelPerTile / wmoPerTile
  for (int z = 0; z < 64; ++z)
  {
    for (int x = 0; x < 64; ++x)
    {
      if (!mTiles[z][x].flags & 1)
      {
        continue;
      }

      // load even the tiles without models in case there are old ones
      // that shouldn't be there to avoid creating new duplicates

      std::stringstream filename;
      filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

      // load the tile without the models
      MapTile tile(x, z, filename.str(), mBigAlpha, false);

      std::map<int, ModelInstance> modelInst;
      std::map<int, WMOInstance> wmoInst;

      for (ModelInstance* instance : modelPerTile[z][x])
      {
        modelInst.emplace(instance->d1, *instance);
      }
      modelPerTile[z][x].clear();

      for (WMOInstance* instance : wmoPerTile[z][x])
      {
        wmoInst.emplace(instance->mUniqueID, *instance);
      }
      wmoPerTile[z][x].clear();

      // save using the models selected beforehand
      std::swap(gWorld->mModelInstances, modelInst);
      std::swap(gWorld->mWMOInstances, wmoInst);
      tile.saveTile(true);
      // restore the original map in World
      std::swap(gWorld->mModelInstances, modelInst);
      std::swap(gWorld->mWMOInstances, wmoInst);
    }
  }

  saveMaxUID();
}

void MapIndex::searchMaxUID()
{
  for (int z = 0; z < 64; ++z)
  {
    for (int x = 0; x < 64; ++x)
    {
      if (!(mTiles[z][x].flags & 1))
      {
        continue;
      }

      std::stringstream filename;
      filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";
      highestGUID = std::max(highestGUID, getHighestGUIDFromFile(filename.str()));
    }
  }
  saveMaxUID();
}

void MapIndex::saveMaxUID()
{
#ifdef USE_MYSQL_UID_STORAGE
  if (Settings::getInstance()->mysql) {
  if (mysql::hasMaxUIDStoredDB(*Settings::getInstance()->mysql, gWorld->mMapId))
  {
	  mysql::updateUIDinDB(*Settings::getInstance()->mysql, gWorld->mMapId, highestGUID);
  }
  else
  {
	  mysql::insertUIDinDB(*Settings::getInstance()->mysql, gWorld->mMapId, highestGUID);
  }
  }
  else
  {
    // save the max UID on the disc
    uid_storage::getInstance()->saveMaxUID(gWorld->mMapId, highestGUID);
  }
#else
  // save the max UID on the disc
  uid_storage::getInstance()->saveMaxUID(gWorld->mMapId, highestGUID);
#endif
}

void MapIndex::loadMaxUID()
{
#ifdef USE_MYSQL_UID_STORAGE
if (Settings::getInstance()->mysql) {
  highestGUID = mysql::getGUIDFromDB(*Settings::getInstance()->mysql, gWorld->mMapId);
}
else
{
  highestGUID = uid_storage::getInstance()->getMaxUID(gWorld->mMapId);
}
#else
  highestGUID = uid_storage::getInstance()->getMaxUID(gWorld->mMapId);
#endif
}
