#include "ChunkWater.h"

#include "MPQ.h"
#include "Liquid.h"
#include "Misc.h"

ChunkWater::ChunkWater(float pX, float pY)
  : x(pX)
  , y(pY)
{}

ChunkWater::~ChunkWater()
{}

void ChunkWater::reloadRendering()
{
  if(!Header.nLayers) return;

  Liquids[0] = new Liquid(Info[0].width, Info[0].height, Vec3D(x, Info[0].minHeight, y));

  MH2O_Tile lTile;
  lTile.mLiquidType = Info[0].LiquidType;
  lTile.mMaximum = Info[0].maxHeight;
  lTile.mMinimum = Info[0].minHeight;
  lTile.mFlags = Info[0].Flags;

  for( int x = 0; x < 9; ++x )
  {
    for( int y = 0; y < 9; ++y )
    {
      lTile.mHeightmap[x][y] = HeightData[0].mHeightValues[x][y];
      lTile.mDepth[x][y] = (HeightData[0].mTransparency[x][y] / 255.0f);
    }
  }

  for(int h=0 ; h < 8; ++h)
  {
    for(int w=0; w < 8; ++w)
    {
      lTile.mRender[w][h] = Render[0].mRender[w][h];
    }
  }

  Liquids[0]->setMH2OData(lTile);
}

void ChunkWater::fromFile(MPQFile &f, size_t basePos)
{
  f.read(&Header, sizeof(MH2O_Header));
  if(!Header.nLayers) return;

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
        Render[k].mRender[w][h] = /*(Header.ofsRenderMask) ? (1 << h) & rMask[w] : */true; //if we have no MH2O_Render structure
        Render[k].fRender[w][h] = /*(Header.ofsRenderMask) ? (1 << h) & fMask[w] : */true;
      }
    }

    //mask
    if(Info[k].ofsInfoMask > 0 && Info[k].height > 0)
    {
      //(Info[k].width * Info[k].height) / 8;
      f.seek(Info[k].ofsInfoMask + basePos);  /*if(numBytes == 0 && (Info[k].width > 0 && Info[k].height > 0)) numBytes = 1;*/

      InfoMask[k] = new char[Info[k].height];
      f.read(&InfoMask[k], Info[k].height);
    }

    for(int h=0 ; h < 9; ++h)
    {
      for(int w=0; w < 9; ++w)
      {
        HeightData[k].mHeightValues[w][h] = Info[k].minHeight;
        HeightData[k].mTransparency[w][h] = 255;
      }
    }


    if(Info[k].ofsHeightMap)// && !(Info[k].Flags & 2)) {
    {
      f.seek(basePos + Info[k].ofsHeightMap);

      if(Info[k].Flags != 2) //if Flags == 2 then there are no Heighmap data...
      {
        for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
        {
          for(int h = Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
          {
            f.read(&HeightData[k].mHeightValues[w][h], sizeof(float));
          }
        }
      }

      for (int w = Info[k].yOffset; w < Info[k].yOffset + Info[k].height + 1; ++w)
      {
        for(int h = Info[k].xOffset; h < Info[k].xOffset + Info[k].width + 1; ++h)
        {
          f.read(&HeightData[k].mTransparency[w][h], sizeof(unsigned char));
        }
      }
    }
  }
}

void ChunkWater::writeHeader(sExtendableArray &lADTFile, int &lCurrentPosition)
{
  lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Header), reinterpret_cast<char*>(&Header));
  lCurrentPosition += sizeof(MH2O_Header);
}

void ChunkWater::writeInfo(sExtendableArray &lADTFile, MH2O_Header *header, size_t basePos, int &lCurrentPosition)
{
  if(!Header.nLayers) return;

  lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Information), reinterpret_cast<char*>(&Info[0])); //insert MH2O_Information
  header->ofsInformation = lCurrentPosition - basePos; //setting offset to this info at the header
  header->nLayers = 1;
  lCurrentPosition += sizeof(MH2O_Information);
}

void ChunkWater::writeData(MH2O_Header *header,  MH2O_Information *info, sExtendableArray &lADTFile, size_t basePos, int &lCurrentPosition)
{
  if(!Header.nLayers) return;

  //info->Flags = 550;

  //render
  //just write this in anycase should be fine
  char wRender[8];
  char fRender[8];

  for(int h=0 ; h < 8; ++h)
  {
    for(int w=0; w < 8; ++w)
    {
      Render[0].mRender[w][h] ? (wRender[w] |= (1<<h)) : (wRender[w] &=~(1<<h)); //render mask
      Render[0].fRender[w][h] ? (fRender[w] |= (1<<h)) : (fRender[w] &=~(1<<h)); //fatigue render mask?
    }
  }

  lADTFile.Insert(lCurrentPosition, 8*sizeof(char), reinterpret_cast<char*>(wRender));
  lADTFile.Insert(lCurrentPosition+8, 8*sizeof(char), reinterpret_cast<char*>(fRender));
  header->ofsRenderMask = lCurrentPosition - basePos;
  lCurrentPosition +=  2*8*sizeof(char);

  //mask
  if(Info[0].ofsInfoMask > 0 && Info[0].height > 0)
  {
    lADTFile.Insert(lCurrentPosition, Info[0].height, reinterpret_cast<char*>(&InfoMask[0]));
    info->ofsInfoMask = lCurrentPosition - basePos;
    lCurrentPosition +=  Info[0].height;
  }

  bool hasHeightmap((Info[0].Flags != 2));

  //HeighData & TransparencyData
  if(hasHeightmap)
  {
    //write a full heightmap here; should be the safest for now
    info->yOffset = 0;
    info->xOffset = 0;
    info->height = 8;
    info->width = 8;
    info->ofsHeightMap = lCurrentPosition - basePos;

    lADTFile.Insert(lCurrentPosition, 9*9*sizeof(float), reinterpret_cast<char*>(HeightData[0].mHeightValues));
    lCurrentPosition +=  9*9*sizeof(float);

    lADTFile.Insert(lCurrentPosition, 9*9*sizeof(unsigned char), reinterpret_cast<char*>(HeightData[0].mTransparency));
    lCurrentPosition += 9*9*sizeof(unsigned char);
  }
  else
  {
    info->ofsHeightMap = 0;
  }

  /*
  bool eFlag = true; //Blizz zips their files here using same bytes at mask and transparency. They move transparency bytes back if some last bytes of previous data matches some first bytes of next data.
  //It is genious I think! We have to do it better =)
  int offs = 1;		 //ADT will be fully correct without this but if we want to make Blizzlike files or better we have to make it work.
  int offs2 =1;
  if(used.Mask[i][j])
    while(eFlag){
      for(int i=0; i<offs; ++i)
        if(lADTFile.GetPointer<unsigned char>(lCurrentPosition-offs)[i]==HeightData[i][j][0].mTransparency[0][i]){
          offs2++;
          break;
        }else{
          eFlag = false;
          break;
        }
      offs=offs2;
    }

  lCurrentPosition-=offs-1;
  */
}

bool ChunkWater::hasLayer(size_t x, size_t y)
{
  if(!Header.nLayers) return false;
  if(x > 8 || y > 8) return false;
  return Render[0].mRender[y][x];
}

void ChunkWater::addLayer(size_t x, size_t y)
{
  if(hasLayer(x,y)) return;
  Render[0].mRender[y][x] = true;
}

void ChunkWater::setHeight(size_t x, size_t y, float height)
{
  if(x > 8 || y > 8) return;
  addLayer(x,y);

  HeightData[0].mHeightValues[y][x] = height;
  HeightData[0].mHeightValues[y+1][x] = height;
  HeightData[0].mHeightValues[y][x+1] = height;
  HeightData[0].mHeightValues[y+1][x+1] = height;

  reloadRendering();
}

void ChunkWater::setTrans(size_t x, size_t y, unsigned char trans)
{
  if(!hasLayer(x,y)) return;

  HeightData[0].mTransparency[y][x]  = trans;
  HeightData[0].mTransparency[y+1][x]  = trans;
  HeightData[0].mTransparency[y][x+1]  = trans;
  HeightData[0].mTransparency[y+1][x+1]  = trans;

  reloadRendering();
}

void ChunkWater::draw()
{
  if(!Header.nLayers) return;
  Liquids[0]->draw();
}
