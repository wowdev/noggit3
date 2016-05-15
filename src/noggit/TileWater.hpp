#pragma once

#include <noggit/ChunkWater.hpp>
#include <noggit/MapHeaders.h>
#include <noggit/mpq/file.h>

#include <boost/optional.hpp>

#include <vector>
#include <memory>

class MapTile;
class Skies;

namespace noggit
{
  class TileWater
  {
  public:
    TileWater (MapTile*, float pXbase, float pZbase);

    TileWater (TileWater const&) = delete;
    TileWater (TileWater&&) = delete;
    TileWater& operator= (TileWater const&) = delete;
    TileWater& operator= (TileWater&&) = delete;

    ChunkWater* getChunk (int x, int y);

    void readFromFile (mpq::file&, size_t basePos);
    void saveToFile (std::vector<char>& adt_file, int& MHDR_Position, int& currentPosition);

    void draw (Skies const*);
    bool hasData();

    void autoGen (int factor);

    void setHeight (float height);
    boost::optional<float> getHeight() const;

    void setTrans (unsigned char);
    boost::optional<unsigned char> getTrans() const;

    void setType (int type);
    boost::optional<int> getType() const;

    void addLayer();
    void addLayer (float height, unsigned char trans);

    void deleteLayer();

  private:
    void reload();

    MapTile* tile;
    std::unique_ptr<ChunkWater> chunks[16][16];

    float xbase;
    float zbase;
  };
}
