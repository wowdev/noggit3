// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <assert.h>
#include <string>
#include <stdint.h>
#include <sstream>
#include <fstream>
#include <ctime>

#include "MapHeaders.h"
#include "MapTile.h"
#include "Misc.h"
#include "tile_index.hpp"

#include <boost/range/iterator_range.hpp>

/*!
\brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
*/
class MapTileEntry
{
private:
	uint32_t flags;
	MapTile* tile;
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
    auto tiles ( std::function<bool (tile_index const&, MapTile*)> pred
               = [] (tile_index const&, MapTile*) { return true; }
               )
  {
    return boost::make_iterator_range
      (tile_iterator<Load> {this, {0, 0}, pred}, tile_iterator<Load>{});
  }

  auto loaded_tiles()
  {
    return tiles<false>
      ([] (tile_index const&, MapTile* tile) { return !!tile; });
  }

  auto tiles_in_range (float x, float z, float radius)
  {
    return tiles<true>
      ( [this, x, z, radius] (tile_index const& index, MapTile*)
        {
          return hasTile(index) && misc::getShortestDist
            (x, z, index.x * TILESIZE, index.z * TILESIZE, TILESIZE) <= radius;
        }
      );
  }

	MapIndex(const std::string& pBasename);
	~MapIndex();

	void enterTile(const tile_index& tile);
	MapTile *loadTile(const tile_index& tile);

	void setChanged(float x, float z);
	void setChanged(const tile_index& tile);
  void setChanged(MapTile* tile);

	void unsetChanged(const tile_index& tile);
	void setFlag(bool to, float x, float z);
	void setWater(bool to, float x, float z);
	int getChanged(const tile_index& tile);

	void saveTile(const tile_index& tile);
	void saveChanged();
	void reloadTile(const tile_index& tile);
	void unloadTiles(const tile_index& tile);	// unloads all tiles more then x adts away from given
	void unloadTile(const tile_index& tile);	// unload given tile
	void markOnDisc(const tile_index& tile, bool mto);
	bool isTileExternal(const tile_index& tile);

	bool hasAGlobalWMO();
	bool hasTile(const tile_index& index) const;
	bool tileLoaded(const tile_index& tile) const;

	bool hasAdt();
	void setAdt(bool value);

	void save();

  uint32_t getHighestGUIDFromFile(const std::string& pFilename);

	MapTile* getTile(const tile_index& tile) const;
  MapTile* getTileAbove(MapTile* tile) const;
  MapTile* getTileLeft(MapTile* tile) const;
	uint32_t getFlag(const tile_index& tile) const;

  void setBigAlpha();
  bool hasBigAlpha() const { return mBigAlpha; }

  uint32_t newGUID();

private:
  bool hasTile(int tileX, int tileZ) const;
  bool tileLoaded(int tileX, int tileZ) const;

	const std::string basename;
	std::string globalWMOName;

	int lastUnloadTime;

	// Is the WDT telling us to use a different alphamap structure.
	bool mBigAlpha;
	bool mHasAGlobalWMO;
	bool noadt;
	bool changed;

	bool autoheight;

	int cx;
	int cz;

  uint32_t highestGUID;

	ENTRY_MODF wmoEntry;
	MPHD mphd;

	// Holding all MapTiles there can be in a World.
	MapTileEntry mTiles[64][64];
};
