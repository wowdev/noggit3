// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h>
#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#ifdef USE_MYSQL_UID_STORAGE
  #include <mysql/mysql.h>
#endif
#include <noggit/map_index.hpp>
#include <noggit/uid_storage.hpp>

#include <QtCore/QSettings>

#include <boost/range/adaptor/map.hpp>

#include <forward_list>

MapIndex::MapIndex (const std::string &pBasename, int map_id, World* world)
  : basename(pBasename)
  , _map_id (map_id)
  , _last_unload_time((clock() / CLOCKS_PER_SEC)) // to not try to unload right away
  , mBigAlpha(false)
  , mHasAGlobalWMO(false)
  , noadt(false)
  , changed(false)
  , _sort_models_by_size_class(false)
  , highestGUID(0)
  , _world (world)
{

  QSettings settings;
  _unload_interval = settings.value("unload_interval", 5).toInt();
  _unload_dist = settings.value("unload_dist", 5).toInt();

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
  _sort_models_by_size_class = mphd.flags & 0x8;

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

      std::stringstream adt_filename;
      adt_filename << "World\\Maps\\" << basename << "\\" << basename << "_" << i << "_" << j << ".adt";

      mTiles[j][i].tile = nullptr;
      mTiles[j][i].onDisc = MPQFile::existsOnDisk(adt_filename.str());

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

void MapIndex::saveall (World* world)
{
  world->wait_for_all_tile_updates();

  saveMaxUID();

  for (MapTile* tile : loaded_tiles())
  {
    tile->saveTile(world);
    tile->changed = false;
  }
}

void MapIndex::save()
{
  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdt";

  //NOGGIT_LOG << "Saving WDT \"" << filename << "\"." << std::endl;

  util::sExtendableArray wdtFile;
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
  f.setBuffer(wdtFile.all_data());
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
  int cx = tile.x;
  int cz = tile.z;

  for (int pz = std::max(cz - 1, 0); pz < std::min(cz + 2, 63); ++pz)
  {
    for (int px = std::max(cx - 1, 0); px < std::min(cx + 2, 63); ++px)
    {
      loadTile(tile_index(px, pz));
    }
  }
}

void MapIndex::update_model_tile(const tile_index& tile, model_update type, uint32_t uid)
{
  if (!hasTile(tile))
  {
    return;
  }

  MapTile* adt = loadTile(tile);
  adt->wait_until_loaded();
  adt->changed = true;

  if (type == model_update::add)
  {
    adt->add_model(uid);
  }
  else if(type == model_update::remove)
  {
    adt->remove_model(uid);
  }
}

void MapIndex::setChanged(const tile_index& tile)
{
  MapTile* mTile = loadTile(tile);

  if (!!mTile)
  {
    mTile->changed = true;
  }
}

void MapIndex::setChanged(MapTile* tile)
{
  setChanged(tile->index);
}

void MapIndex::unsetChanged(const tile_index& tile)
{
  // change the changed flag of the map tile
  if (hasTile(tile))
  {
    mTiles[tile.z][tile.x].tile->changed = false;
  }
}

bool MapIndex::has_unsaved_changes(const tile_index& tile) const
{
  return (tileLoaded(tile) ? getTile(tile)->changed.load() : false);
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

MapTile* MapIndex::loadTile(const tile_index& tile, bool reloading)
{
  if (!hasTile(tile))
  {
    return nullptr;
  }

  if (tileLoaded(tile) || tileAwaitingLoading(tile))
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

  mTiles[tile.z][tile.x].tile = std::make_unique<MapTile> (tile.x, tile.z, filename.str(), mBigAlpha, true, use_mclq_green_lava(), reloading, _world);

  MapTile* adt = mTiles[tile.z][tile.x].tile.get();

  AsyncLoader::instance().queue_for_load(adt);

  return adt;
}

void MapIndex::reloadTile(const tile_index& tile)
{
  if (tileLoaded(tile))
  {
    mTiles[tile.z][tile.x].tile.reset();
    loadTile(tile, true);
  }
}

void MapIndex::unloadTiles(const tile_index& tile)
{
  if (((clock() / CLOCKS_PER_SEC) - _last_unload_time) > _unload_interval)
  {
    for (MapTile* adt : loaded_tiles())
    {
      if (tile.dist(adt->index) > _unload_dist)
      {
        //Only unload adts not marked to save
        if (!adt->changed.load())
        {
          unloadTile(adt->index);
        }
      }
    }

    _last_unload_time = clock() / CLOCKS_PER_SEC;
  }
}

void MapIndex::unloadTile(const tile_index& tile)
{
  // unloads a tile with givn cords
  if (tileLoaded(tile))
  {
    mTiles[tile.z][tile.x].tile = nullptr;
    NOGGIT_LOG << "Unload Tile " << tile.x << "-" << tile.z << std::endl;
  }
}

void MapIndex::markOnDisc(const tile_index& tile, bool mto)
{
  if(tile.is_valid())
  {
    mTiles[tile.z][tile.x].onDisc = mto;
  }
}

bool MapIndex::isTileExternal(const tile_index& tile) const
{
  // is onDisc
  return tile.is_valid() && mTiles[tile.z][tile.x].onDisc;
}

void MapIndex::saveTile(const tile_index& tile, World* world)
{
  world->wait_for_all_tile_updates();

	// save given tile
	if (tileLoaded(tile))
	{
    saveMaxUID();
		mTiles[tile.z][tile.x].tile->saveTile(world);
	}
}

void MapIndex::saveChanged (World* world)
{
  world->wait_for_all_tile_updates();

  if (changed)
  {
    save();
  }

  saveMaxUID();

  for (MapTile* tile : loaded_tiles())
  {
    if (tile->changed.load())
    {
      tile->saveTile(world);
      tile->changed = false;
    }
  }
}

bool MapIndex::hasAGlobalWMO()
{
  return mHasAGlobalWMO;
}


bool MapIndex::hasTile(const tile_index& tile) const
{
  return tile.is_valid() && (mTiles[tile.z][tile.x].flags & 1);
}

bool MapIndex::tileAwaitingLoading(const tile_index& tile) const
{
  return hasTile(tile) && mTiles[tile.z][tile.x].tile && !mTiles[tile.z][tile.x].tile->finishedLoading();
}

bool MapIndex::tileLoaded(const tile_index& tile) const
{
  return hasTile(tile) && mTiles[tile.z][tile.x].tile && mTiles[tile.z][tile.x].tile->finishedLoading();
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
  return (tile.is_valid() ? mTiles[tile.z][tile.x].tile.get() : nullptr);
}

MapTile* MapIndex::getTileAbove(MapTile* tile) const
{
  tile_index above(tile->index.x, tile->index.z - 1);
  if (tile->index.z == 0 || (!tileLoaded(above) && !tileAwaitingLoading(above)))
  {
    return nullptr;
  }

  MapTile* tile_above = mTiles[tile->index.z - 1][tile->index.x].tile.get();
  tile_above->wait_until_loaded();

  return tile_above;
}

MapTile* MapIndex::getTileLeft(MapTile* tile) const
{
  tile_index left(tile->index.x - 1, tile->index.z);
  if (tile->index.x == 0 || (!tileLoaded(left) && !tileAwaitingLoading(left)))
  {
    return nullptr;
  }

  MapTile* tile_left = mTiles[tile->index.z][tile->index.x - 1].tile.get();
  tile_left->wait_until_loaded();

  return tile_left;
}

uint32_t MapIndex::getFlag(const tile_index& tile) const
{
  return (tile.is_valid() ? mTiles[tile.z][tile.x].flags : 0);
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

uint32_t MapIndex::newGUID()
{
  std::unique_lock<std::mutex> lock (_mutex);

#ifdef USE_MYSQL_UID_STORAGE
  QSettings settings;

  if (settings->value ("project/mysql/enabled", false).toBool())
  {
    mysql::updateUIDinDB(_map_id, highestGUID + 1); // update the highest uid in db, note that if the user don't save these uid won't be used (not really a problem tho)
  }
#endif
  return ++highestGUID;
}

uid_fix_status MapIndex::fixUIDs (World* world, bool cancel_on_model_loading_error)
{
  // pre-cond: mTiles[z][x].flags are set

  // unload any previously loaded tile, although there shouldn't be as
  // the fix is executed before loading the map
  for (int z = 0; z < 64; ++z)
  {
    for (int x = 0; x < 64; ++x)
    {
      if (mTiles[z][x].tile)
      {
        MapTile* tile = mTiles[z][x].tile.get();

        // don't unload half loaded tiles
        tile->wait_until_loaded();

        unloadTile(tile->index);
      }
    }
  }

  _uid_fix_all_in_progress = true;

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
            && misc::float_equals(mddf.pos[0], entry.pos[0])
            && misc::float_equals(mddf.pos[1], entry.pos[1])
            && misc::float_equals(mddf.pos[2], entry.pos[2])
            && misc::float_equals(mddf.rot[0], entry.rot[0])
            && misc::float_equals(mddf.rot[1], entry.rot[1])
            && misc::float_equals(mddf.rot[2], entry.rot[2])
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
            && misc::float_equals(modf.pos[0], entry.pos[0])
            && misc::float_equals(modf.pos[1], entry.pos[1])
            && misc::float_equals(modf.pos[2], entry.pos[2])
            && misc::float_equals(modf.rot[0], entry.rot[0])
            && misc::float_equals(modf.rot[1], entry.rot[1])
            && misc::float_equals(modf.rot[2], entry.rot[2])
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
  highestGUID = 0;

  std::map<std::size_t, std::map<std::size_t, std::forward_list<std::uint32_t>>> uids_per_tile;

  bool loading_error = false;

  for (ModelInstance& instance : models)
  {
    instance.uid = highestGUID++;
    instance.model->wait_until_loaded();

    loading_error |= instance.model->loading_failed();

    auto const& extents(instance.extents());

    // to avoid going outside of bound
    std::size_t sx = std::max((std::size_t)(extents[0].x / TILESIZE), (std::size_t)0);
    std::size_t sz = std::max((std::size_t)(extents[0].z / TILESIZE), (std::size_t)0);
    std::size_t ex = std::min((std::size_t)(extents[1].x / TILESIZE), (std::size_t)63);
    std::size_t ez = std::min((std::size_t)(extents[1].z / TILESIZE), (std::size_t)63);

    auto const real_uid (world->add_model_instance (std::move(instance), false));

    for (std::size_t z = sz; z <= ez; ++z)
    {
      for (std::size_t x = sx; x <= ex; ++x)
      {
        uids_per_tile[z][x].push_front (real_uid);
      }
    }
  }

  models.clear();

  for (WMOInstance& instance : wmos)
  {
    instance.mUniqueID = highestGUID++;
    // no need to check if the loading is finished since the extents are stored inside the adt
    // to avoid going outside of bound
    std::size_t sx = std::max((std::size_t)(instance.extents[0].x / TILESIZE), (std::size_t)0);
    std::size_t sz = std::max((std::size_t)(instance.extents[0].z / TILESIZE), (std::size_t)0);
    std::size_t ex = std::min((std::size_t)(instance.extents[1].x / TILESIZE), (std::size_t)63);
    std::size_t ez = std::min((std::size_t)(instance.extents[1].z / TILESIZE), (std::size_t)63);

    auto const real_uid (world->add_wmo_instance (std::move(instance), false));

    for (std::size_t z = sz; z <= ez; ++z)
    {
      for (std::size_t x = sx; x <= ex; ++x)
      {
        uids_per_tile[z][x].push_front (real_uid);
      }
    }
  }

  wmos.clear();

  if (cancel_on_model_loading_error && loading_error)
  {
    return uid_fix_status::failed;
  }

  // load each tile without the models and
  // save them with the models with the new uids
  for (int z = 0; z < 64; ++z)
  {
    for (int x = 0; x < 64; ++x)
    {
      if (!(mTiles[z][x].flags & 1))
      {
        continue;
      }

      // load even the tiles without models in case there are old ones
      // that shouldn't be there to avoid creating new duplicates

      std::stringstream filename;
      filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

      // load the tile without the models
      MapTile tile(x, z, filename.str(), mBigAlpha, false, use_mclq_green_lava(), false, world, tile_mode::uid_fix_all);
      tile.finishLoading();

      // add the uids to the tile to be able to save the models
      // which have been loaded in world earlier
      for (std::uint32_t uid : uids_per_tile[z][x])
      {
        tile.add_model(uid);
      }

      tile.saveTile(world);
    }
  }

  // override the db highest uid if used
  saveMaxUID();

  _uid_fix_all_in_progress = false;

  // force instances unloading
  world->unload_every_model_and_wmo_instance();

  return loading_error ? uid_fix_status::done_with_errors : uid_fix_status::done;
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
  QSettings settings;

  if (settings->value ("project/mysql/enabled", false).toBool())
  {
    if (mysql::hasMaxUIDStoredDB(_map_id))
    {
	    mysql::updateUIDinDB(_map_id, highestGUID);
    }
    else
    {
	    mysql::insertUIDinDB(_map_id, highestGUID);
    }
  }
#endif
  // save the max UID on the disk (always save to sync with the db if used
  uid_storage::saveMaxUID (_map_id, highestGUID);
}

void MapIndex::loadMaxUID()
{
  highestGUID = uid_storage::getMaxUID (_map_id);
#ifdef USE_MYSQL_UID_STORAGE
  QSettings settings;

  if (settings->value ("project/mysql/enabled", false).toBool())
  {
    highestGUID = std::max(mysql::getGUIDFromDB(map_id), highestGUID);
    // save to make sure the db and disk uid are synced
    saveMaxUID();
  }
#endif
}

MapIndex::tile_range<false> MapIndex::loaded_tiles()
{
  return tiles<false>
    ([] (tile_index const&, MapTile* tile) { return !!tile && tile->finishedLoading(); });
}

MapIndex::tile_range<true> MapIndex::tiles_in_range (math::vector_3d const& pos, float radius)
{
  return tiles<true>
    ( [this, pos, radius] (tile_index const& index, MapTile*)
      {
        return hasTile(index) && misc::getShortestDist
          (pos.x, pos.z, index.x * TILESIZE, index.z * TILESIZE, TILESIZE) <= radius;
      }
    );
}

MapIndex::tile_range<true> MapIndex::tiles_between (math::vector_3d const& pos1, math::vector_3d const& pos2)
{
  return tiles<true>
    ( [this, pos1, pos2] (tile_index const& index, MapTile*)
      {
        auto minX = index.x*TILESIZE;
        auto maxX = index.x*TILESIZE+TILESIZE;
        auto minZ = index.z*TILESIZE;
        auto maxZ = index.z*TILESIZE+TILESIZE;

        return hasTile(index) &&
          minX <= pos2.x && maxX >= pos1.x &&
          minZ <= pos2.z && maxZ >= pos1.z;
        }
    );
}
