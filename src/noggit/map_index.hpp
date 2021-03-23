// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/map_enums.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/tile_index.hpp>

#include <boost/range/iterator_range.hpp>

#include <cassert>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>


enum class uid_fix_status
{
  done,
  done_with_errors,
  failed
};

/*!
\brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
*/
class MapTileEntry
{
private:
  uint32_t flags;
  std::unique_ptr<MapTile> tile;
  bool onDisc;


  MapTileEntry() : flags(0), tile(nullptr) {}

  friend class MapIndex;
};

class MapIndex
{
public:
  template<bool Load>
    struct tile_iterator
      : std::iterator<std::forward_iterator_tag, MapTile*, std::ptrdiff_t, MapTile**, MapTile* const&>
  {
    template<typename Pred>
      tile_iterator (MapIndex* index, tile_index tile, Pred pred)
        : _index (index)
        , _tile (std::move (tile))
        , _pred (std::move (pred))
    {
      if (!_pred (_tile, _index->getTile (_tile)))
      {
        ++(*this);
      }
    }

    tile_iterator()
      : _index (nullptr)
      , _tile (0, 0)
      , _pred ([] (tile_index const&, MapTile*) { return false; })
    {}

    bool operator== (tile_iterator const& other) const
    {
      return std::tie (_index, _tile) == std::tie (other._index, other._tile);
    }
    bool operator!= (tile_iterator const& other) const
    {
      return !operator== (other);
    }

    tile_iterator& operator++()
    {
      do
      {
        ++_tile.x;

        if (_tile.x == 64)
        {
          _tile.x = 0;
          ++_tile.z;

          if (_tile.z == 64)
          {
            _tile.x = 0;
            _tile.z = 0;
            _index = nullptr;
            break;
          }
        }
      }
      while (!_pred (_tile, _index->getTile (_tile)));

      return *this;
    }

    tile_iterator operator++ (int) const
    {
      tile_iterator it (*this);
      ++it;
      return it;
    }

    MapTile* operator*() const
    {
      return Load ? _index->loadTile (_tile) : _index->getTile (_tile);
    }
    MapTile* operator->() const
    {
      return operator*();
    }

    MapIndex* _index;
    tile_index _tile;
    std::function<bool (tile_index const&, MapTile*)> _pred;
  };

  template<bool Load>
    using tile_range = boost::iterator_range<tile_iterator<Load>>;

  template<bool Load>
    auto tiles ( std::function<bool (tile_index const&, MapTile*)> pred
               = [] (tile_index const&, MapTile*) { return true; }
               )
  {
    return boost::make_iterator_range
      (tile_iterator<Load> {this, {0, 0}, pred}, tile_iterator<Load>{});
  }

  tile_range<false> loaded_tiles();
  tile_range<true> tiles_in_range (math::vector_3d const& pos, float radius);
  //! inclusive
  tile_range<true> tiles_between (math::vector_3d const& pos1, math::vector_3d const& pos2);

  MapIndex(const std::string& pBasename, int map_id, World*);

  void enterTile(const tile_index& tile);
  MapTile *loadTile(const tile_index& tile, bool reloading = false);

  void update_model_tile(const tile_index& tile, model_update type, uint32_t uid);

  void setChanged(const tile_index& tile);
  void setChanged(MapTile* tile);

  void unsetChanged(const tile_index& tile);
  void setFlag(bool to, math::vector_3d const& pos, uint32_t flag);
  bool has_unsaved_changes(const tile_index& tile) const;

  void saveTile(const tile_index& tile, World*);
  void saveChanged (World*);
  void reloadTile(const tile_index& tile);
  void unloadTiles(const tile_index& tile);  // unloads all tiles more then x adts away from given
  void unloadTile(const tile_index& tile);  // unload given tile
  void markOnDisc(const tile_index& tile, bool mto);
  bool isTileExternal(const tile_index& tile) const;

  bool hasAGlobalWMO();
  bool hasTile(const tile_index& index) const;
  bool tileAwaitingLoading(const tile_index& tile) const;
  bool tileLoaded(const tile_index& tile) const;

  bool hasAdt();
  void setAdt(bool value);

  void save();
  void saveall (World*);

  MapTile* getTile(const tile_index& tile) const;
  MapTile* getTileAbove(MapTile* tile) const;
  MapTile* getTileLeft(MapTile* tile) const;
  uint32_t getFlag(const tile_index& tile) const;

  void convert_alphamap(bool to_big_alpha);
  bool hasBigAlpha() const { return mBigAlpha; }

  bool sort_models_by_size_class() const { return _sort_models_by_size_class; }

  uint32_t newGUID();

  uid_fix_status fixUIDs (World*, bool);
  void searchMaxUID();
  void saveMaxUID();
  void loadMaxUID();

  // todo: find out how wow choose to use the green lava in outland
  inline bool use_mclq_green_lava() const
  {
    return _map_id == 530;
  }

  bool uid_fix_all_in_progress() const
  {
    return _uid_fix_all_in_progress;
  }

private:
	uint32_t getHighestGUIDFromFile(const std::string& pFilename) const;

  bool _uid_fix_all_in_progress = false;

  const std::string basename;

public:
  int const _map_id;

private:
  std::string globalWMOName;

  int _last_unload_time;
  int _unload_interval;
  int _unload_dist;

  // Is the WDT telling us to use a different alphamap structure.
  bool mBigAlpha;
  bool mHasAGlobalWMO;
  bool noadt;
  bool changed;
  bool _sort_models_by_size_class;

  bool autoheight;

  uint32_t highestGUID;

  ENTRY_MODF wmoEntry;
  MPHD mphd;

  // Holding all MapTiles there can be in a World.
  MapTileEntry mTiles[64][64];

  //! \todo REMOVE!
  World* _world;

  std::mutex _mutex;
};
