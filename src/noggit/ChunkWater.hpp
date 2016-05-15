#pragma once

#include <noggit/MapHeaders.h>
#include <noggit/mpq/file.h>

#include <vector>

class Liquid;
class MapChunk;
class Skies;

namespace noggit
{
  class ChunkWater
  {
  public:
    ChunkWater (float pX, float pY);

    void fromFile (mpq::file& f, size_t basePos);
    void reloadRendering();
    void draw (Skies const*);

    void writeHeader (std::vector<char>& lADTFile, int& lCurrentPosition);
    void writeInfo (std::vector<char>& lADTFile, size_t basePos, int& lCurrentPosition);
    void writeData (size_t offHeader, std::vector<char>& lADTFile, size_t basePos, int& lCurrentPosition);

    void autoGen (MapChunk* chunk, int factor);

    void setHeight (float height);
    void setHeight (size_t x, size_t y, float height);
    float getHeight();
    float getHeight (size_t x, size_t y);

    void setTrans (unsigned char trans);
    void setTrans (size_t x, size_t y, unsigned char trans);
    unsigned char getTrans();
    unsigned char getTrans (size_t x, size_t y);

    void setType (int type);
    int getType();

    void addLayer();
    void addLayer (size_t x, size_t y);

    void deleteLayer();
    void deleteLayer (size_t x, size_t y);

    bool hasData();

    void CropWater(MapChunk* chunkTerrain);
    void DelLayer();

  private:
    bool hasLayer (size_t x, size_t y);

    MH2O_Header Header;
    MH2O_Information Info;
    MH2O_HeightMask HeightData;
    MH2O_Render Render;

    Liquid* Liquids;

    bool existsTable[8][8];

    uint8_t InfoMask[8];

    float x, y;
  };
}
