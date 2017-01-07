#ifndef MAPINDEX_H
#define MAPINDEX_H

#include <assert.h>
#include <string>
#include <stdint.h>
#include <sstream>
#include <fstream>
#include <ctime>

#include "MapHeaders.h"
#include "Vec3D.h"

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

struct tile_index
{
  tile_index(const Vec3D& pos) : tile_index(pos.x / TILESIZE, pos.z / TILESIZE) { }
  tile_index(std::size_t tileX, std::size_t tileZ) : x(tileX), z(tileZ)
  {  
    assert(x < 64);
    assert(z < 64);
  }

  int x;
  int z;
};

class MapIndex
{
public:
  struct tiles_in_range
  {
    struct tiles_in_range_iterator
    {
      tiles_in_range_iterator(MapIndex* map_index, tiles_in_range const* range, std::size_t x, std::size_t y)
        : _map_index(map_index)
        , _range(range)
        , _x(x)
        , _y(y)
      {
        if (_y != _range->_end_y && !has_tile())
        {
          ++(*this);
        }
      }

      bool operator!= (tiles_in_range_iterator const& other) const
      {
        //! \note end() only uses _y.
        return _y != other._y;
      }

      tiles_in_range_iterator& operator++()
      {
        if (_y == _range->_end_y)
        {
          return *this;
        }

        do
        {
          _x++;
          if (_x == _range->_end_x)
          {
            _y++;
            _x = _range->_start_x;
          }
        } while (_y != _range->_end_y && !has_tile());

        return *this;
      }

      MapTile* operator*() const
      {
        return _map_index->loadTile(make_index());
      }

      MapTile* operator->() const
      {
        return operator*();
      }

      tile_index make_index() const
      {
        return tile_index(_x, _y);
      }

      bool has_tile() const
      {
        return _map_index->hasTile(make_index());
      }      

      MapIndex* _map_index;
      tiles_in_range const* _range;
      std::size_t _x;
      std::size_t _y;
    };

    MapIndex* _map_index;
    std::size_t _start_x;
    std::size_t _start_y;
    std::size_t _end_x;
    std::size_t _end_y;

    tiles_in_range_iterator begin() const
    {
      return { _map_index, this, _start_x, _start_y };
    }
    tiles_in_range_iterator end() const
    {
      return{ nullptr, this, _end_x, _end_y };
    }
  };

  tiles_in_range tiles_in_range(float x, float z, float radius)
  {
    return{ this
          , (std::size_t)(std::max(0.0f, (x - radius)) / TILESIZE)
          , (std::size_t)(std::max(0.0f, (z - radius)) / TILESIZE)
          , (std::size_t)((x + radius + TILESIZE) / TILESIZE)
          , (std::size_t)((z + radius + TILESIZE) / TILESIZE)
          };
  }

  struct loaded_tiles
  {
    struct loaded_tile_iterator
    {
      loaded_tile_iterator(MapIndex const* map_index, std::size_t x, std::size_t y)
        : _map_index(map_index)
        , _x(x)
        , _y(y)
      {
        if (_y != 64 && !is_loaded())
        {
          ++(*this);
        }
      }

      bool operator!= (loaded_tile_iterator const& other) const
      {
        //! \note end() only uses _y.
        return _y != other._y;
      }

      loaded_tile_iterator& operator++()
      {
        if (_y == 64)
        {
          return *this;
        }

        do
        {
          _x = (_x + 1) % 64;
          _y += _x == 0;
        } while (_y != 64 && !is_loaded());

        return *this;
      }

      MapTile* operator*() const
      {
        return _map_index->getTile(make_index());
      }

      MapTile* operator->() const
      {
        return operator*();
      }

      tile_index make_index() const
      {
        return tile_index(_x, _y);
      }

      bool is_loaded() const
      {
        return _map_index->tileLoaded(make_index());
      }

      MapIndex const* _map_index;
      std::size_t _x;
      std::size_t _y;
    };

    MapIndex const* _map_index;

    loaded_tile_iterator begin() const
    {
      return{ _map_index, 0, 0 };
    }
    loaded_tile_iterator end() const
    {
      return{ nullptr, 64, 64 };
    }
  };

  loaded_tiles loaded_tiles() const
  {
    return{ this };
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
	uint32_t getFlag(const tile_index& tile) const;

  void setBigAlpha();
  bool hasBigAlpha() const { return mBigAlpha; }

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

#endif //MAPINDEX_H
