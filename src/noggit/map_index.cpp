#include "map_index.hpp"

#include <QString>
#include <cassert>

#include <math/bounded_nearest.h>

#include <noggit/MapTile.h>
#include <noggit/Log.h>
#include <noggit/mpq/file.h>

namespace noggit
{
  map_index::MapTileEntry::MapTileEntry()
    : flags(0)
    , tile(nullptr)
  {}

  map_index::map_index(World *world, const std::string &basename)
    : _basename(basename)
    , _big_alpha(false)
    , _world(world)
  {
    const QString filename(QString("World\\Maps\\%1\\%1.wdt")
      .arg(QString::fromStdString(_basename))
    );

    noggit::mpq::file file(filename);
    uint32_t fourcc;

    // - MVER ----------------------------------------------

    uint32_t version;

    file.read(&fourcc, 4);
    file.seekRelative(4);
    file.read(&version, 4);

    //! \todo find the correct version of WDT files.
    assert(fourcc == 'MVER' && version == 18);

    // - MHDR ----------------------------------------------

    uint32_t flags;

    file.read(&fourcc, 4);
    file.seekRelative(4);

    assert(fourcc == 'MPHD');

    file.read(&flags, 4);
    file.seekRelative(4 * 7);

    //! \note This indicates a global WMO, which we don't want to edit, as Noggit
    //        is an ADT based map editor.
    assert(!(flags & 1));

    _big_alpha = flags & 4;

    // - MAIN ----------------------------------------------

    file.read(&fourcc, 4);
    file.seekRelative(4);

    assert(fourcc == 'MAIN');

    for (int j = 0; j < 64; ++j)
    {
      for (int i = 0; i < 64; ++i)
      {
        _tile_got_modified[j][i] = false;

        file.read(&_tiles[j][i].flags, 4);
        file.seekRelative(4);
      }
    }

    // -----------------------------------------------------

    file.close();
  }

  bool map_index::ok_tile(int z, int x)
  {
    return !(z < 0 || x < 0 || z > 64 || x > 64);
  }

  bool map_index::has_tile(int pZ, int pX) const
  {
    return ok_tile(pZ, pX) && (_tiles[pZ][pX].flags & 1);
  }

  void map_index::load_tiles_around(const size_t& x
    , const size_t& z
    , const size_t& distance
  )
  {
    for (size_t i(std::max<size_t>(z - distance, 0))
      ; i < std::min<size_t>(z + distance, 64)
      ; ++i
      )
    {
      for (size_t j(std::max<size_t>(x - distance, 0))
        ; j < std::min<size_t>(x + distance, 64)
        ; ++j
        )
      {
        if (has_tile(i, j))
        {
          load_tile(i, j);
        }
      }
    }
  }

  void map_index::reload_tile(int x, int z)
  {
    if (tile_loaded(z, x))
    {
      _tiles[z][x].tile.reset();

      load_tile(z, x);
    }
  }

  void map_index::save_tile(int x, int z) const
  {
    if (tile_loaded(z, x))
    {
      _tiles[z][x].tile->saveTile(_world->mModelInstances.begin()
                                , _world->mModelInstances.end()
                                , _world->mWMOInstances.begin()
                                , _world->mWMOInstances.end()
                                );
    }
  }

  void map_index::save_tile_cata(int x, int z) const
  {
    if (tile_loaded(z, x))
    {
      _tiles[z][x].tile->saveTileCata(_world->mModelInstances.begin()
                                    , _world->mModelInstances.end()
                                    , _world->mWMOInstances.begin()
                                    , _world->mWMOInstances.end()
                                    );
    }
  }

  void map_index::save_changed()
  {
    // save all changed tiles
    for (int j = 0; j < 64; ++j)
    {
      for (int i = 0; i < 64; ++i)
      {
        if (tile_loaded(j, i))
        {
          if (get_changed(j, i))
          {
            _tiles[j][i].tile->saveTile(_world->mModelInstances.begin()
                                      , _world->mModelInstances.end()
                                      , _world->mWMOInstances.begin()
                                      , _world->mWMOInstances.end()
                                      );
            unset_changed(j, i);
          }
        }
      }
    }

  }

  inline bool map_index::tile_loaded(int z, int x) const
  {
    return has_tile(z, x) && _tiles[z][x].tile;
  }

  MapTile* map_index::load_tile(int z, int x)
  {
    if (!has_tile(z, x))
    {
      return nullptr;
    }

    if (tile_loaded(z, x))
    {
      return _tiles[z][x].tile.get();
    }

    const QString filename
    (QString("World\\Maps\\%1\\%1_%2_%3.adt")
      .arg(QString::fromStdString(_basename))
      .arg(x)
      .arg(z)
    );

    if (!noggit::mpq::file::exists(filename))
    {
      LogError << "The requested tile \"" << qPrintable(filename) << "\" does not exist! Oo" << std::endl;
      return nullptr;
    }

    _tiles[z][x].tile.reset(new MapTile(_world, x, z, filename.toStdString(), _big_alpha));
    return _tiles[z][x].tile.get();
  }

  MapTile* map_index::tile(int z, int x) const
  {
    assert(ok_tile(x, z));
    return _tiles[z][x].tile.get();
  }

  static int tile_below_camera(const float& position)
  {
    return ::math::bounded_nearest<int>((position - (TILESIZE / 2)) / TILESIZE);
  }

  void map_index::set_changed(float x, float z)
  {
    const int column(tile_below_camera(z));
    const int row(tile_below_camera(x));

    set_changed(column, row);
  }

  void map_index::set_changed(int x, int z)
  {
    assert(ok_tile(x, z));
    _tile_got_modified[x][z] = true;
  }

  void map_index::unset_changed(int x, int z)
  {
    assert(ok_tile(x, z));
    _tile_got_modified[x][z] = false;
  }

  bool map_index::get_changed(int x, int z) const
  {
    assert(ok_tile(x, z));
    return _tile_got_modified[x][z];
  }


}
