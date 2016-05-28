#pragma once

#include <string>
#include <memory>

class MapTile;
class World;

namespace noggit
{
  class map_index
  {
  private:
    /*!
    \brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing.
           This is private and for the class World only.
    */
    struct MapTileEntry
    {
      MapTileEntry();

      uint32_t flags;
      std::unique_ptr<MapTile> tile;
    };

  public:
    //! \todo: i do not like this dep to world
    map_index(World *world, const std::string &basename);

    //! \todo: maybe not use raw pointers?
    MapTile *load_tile(int z, int x);
    MapTile *tile(int z, int x) const;

    bool tile_loaded(int x, int z) const;
    bool has_tile(int pX, int pZ) const;

    void load_tiles_around(const size_t& x
      , const size_t& z
      , const size_t& distance
    );
    void reload_tile(int x, int z);

    void save_tile(int x, int z) const;
    void save_tile_cata(int x, int z) const;
    void save_changed();

    void set_changed(float x, float z);
    void set_changed(int x, int z);
    void unset_changed(int x, int z);
    bool get_changed(int x, int z) const;

    static bool ok_tile(int z, int x);

  private:
    World* _world;
    const std::string _basename;

    //! Holding all MapTiles there can be in a World.
    MapTileEntry _tiles[64][64];

    //! Is the WDT telling us to use a bigger alphamap (64*64) and single pass rendering.
    bool _big_alpha;

    bool _tile_got_modified[64][64];
  };
}