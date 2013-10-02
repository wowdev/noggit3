#include "ChunkWater.h"

#include "MPQ.h"
#include "Liquid.h"

ChunkWater::ChunkWater(size_t pX, size_t pY)
  : x(pX)
  , y(pY)
{}

ChunkWater::~ChunkWater()
{}

void ChunkWater::reloadRendering()
{
  Liquids[0] = new Liquid(Info[0].width, Info[0].height, Vec3D(x, Info[0].minHeight, y));

  MH2O_Tile lTile;
  lTile.mLiquidType = Info[0].LiquidType;
  lTile.mMaximum = Info[0].maxHeight;
  lTile.mMinimum = Info[0].minHeight;
  lTile.mFlags = Info[0].Flags;

  for( int x = 0; x < 9; ++x ) {
    for( int y = 0; y < 9; ++y ) {
      lTile.mHeightmap[x][y] = (HeightData[0].mHeightValues[x][y]>0)? (HeightData[0].mHeightValues[x][y]) : lTile.mMinimum;
      lTile.mDepth[x][y] = (HeightData[0].mTransparency[x][y] / 255.0f);
    }
  }

  for(int h=0 ; h < 8; ++h) {
    for(int w=0; w < 8; ++w) {
      lTile.mRender[w][h] = (HeightData[0].mHeightValues[w][h]>0)?HeightData[0].mHeightValues[w][h] : Render[0].mRender[w][h];
    }
  }
  Liquids[0]->setMH2OData(lTile);
}

void ChunkWater::fromFile(MPQFile &f, size_t basePos)
{
  f.read(&Header, sizeof(MH2O_Header));

  if(!Header.nLayers)
  {
    Info[0] = MH2O_Information(); //chunk has no water
    return;
  }

  for(int k=0; k < Header.nLayers; ++k)
  {
    if(k > 0) break; //Temporary only for 1 layer

    //info
    f.seek(basePos + Header.ofsInformation);
    f.read(&Info[k], sizeof(MH2O_Information));

    char rMask[8];
    char fMask[8];

    //render
    if(Header.ofsRenderMask)
    {
      f.seek(basePos + Header.ofsRenderMask);
      f.read(rMask,sizeof(rMask));
      f.read(fMask,sizeof(fMask));
    }

    for(int h=0 ; h < 8; ++h)
    {
      for(int w=0; w < 8; ++w)
      {
        Render[k].mRender[w][h] = (Header.ofsRenderMask) ? (1 << h) & rMask[w] : true; //if we have no MH2O_Render structure
        Render[k].fRender[w][h] = (Header.ofsRenderMask) ? (1 << h) & fMask[w] : true;
      }
    }

    //mask
    if(Info[k].ofsInfoMask > 0)
    {
      int numBytes = Info[k].height;//(Info[k].width * Info[k].height) / 8;
      f.seek(Info[k].ofsInfoMask + basePos);
      /*if(numBytes == 0 && (Info[k].width > 0 && Info[k].height > 0))
                                    numBytes = 1;*/
      if(numBytes>0)
      {
        InfoMask[k] = new char[numBytes];
        f.read(&InfoMask[k], numBytes);
      }
    }


    if(Info[k].ofsHeightMap)// && !(Info[k].Flags & 2)) {
    {
      f.seek(basePos + Info[k].ofsHeightMap);

      if(Info[k].Flags != 2) //if Flags == 2 then there are no Heighmap data...
      {
        for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
        {
          for(int h=Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
          {
            f.read(&HeightData[k].mHeightValues[w][h], sizeof(float));
          }
        }
      }

      for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
      {
        for(int h=Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
        {
          f.read(&HeightData[k].mTransparency[w][h], sizeof(unsigned char));
        }
      }
    }
  }
}

void ChunkWater::draw()
{
  if(!Header.nLayers) return;
  Liquids[0]->draw();
}
