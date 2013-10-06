#include "TileWater.h"

#include "Log.h"
#include "Misc.h"
#include "ChunkWater.h"

TileWater::TileWater(float pXbase, float pZbase)
  : xbase(pXbase)
  , zbase(pZbase)
  , hasData(false)
{}

TileWater::~TileWater(void)
{}

void TileWater::readFromFile(MPQFile &theFile, size_t basePos)
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      theFile.seek(basePos + (i * 16 + j) * sizeof(MH2O_Header));
      chunks[i][j] = new ChunkWater(xbase + CHUNKSIZE * j, zbase + CHUNKSIZE * i);
      chunks[i][j]->fromFile(theFile, basePos);
    }
  }
  hasData = true;
  reload();
}

void TileWater::reload()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->reloadRendering();
    }
  }
}

void TileWater::draw()
{
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      chunks[i][j]->draw();
    }
  }
}

void TileWater::saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition)
{
  if(!hasData) return;
  size_t ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header
  lADTFile.Extend(8); // we need 8 empty bytes
  lCurrentPosition = ofsW;

  //writing MH2O_Header
  for(int i = 0; i < 16; ++i)
  {
    for(int j = 0; j < 16; ++j)
    {
      //there is always a bloody header
      chunks[i][j]->writeHeader(lADTFile, lCurrentPosition);
    }
  }

  //writing MH2O_Information
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      MH2O_Header *header(lADTFile.GetPointer<MH2O_Header>(ofsW + (i * 16 + j) * sizeof(MH2O_Header)));
      chunks[i][j]->writeInfo(lADTFile, header, ofsW, lCurrentPosition); //let chunk check if there is info!
    }
  }

  //writing other Info
  for(int i = 0; i < 16; ++i)
  {
    for(int j = 0; j < 16; ++j)
    {
      MH2O_Header *header(lADTFile.GetPointer<MH2O_Header>(ofsW + (i * 16 + j) * sizeof(MH2O_Header)));
      MH2O_Information *info(lADTFile.GetPointer<MH2O_Information>(ofsW + header->ofsInformation));

      chunks[i][j]->writeData(header, info, lADTFile, ofsW, lCurrentPosition);
    }
  }

  SetChunkHeader(lADTFile, ofsW - 8, 'MH2O', lCurrentPosition - ofsW);
  lCurrentPosition += 8;
}

// following functions change the full ADT
void TileWater::setLevel(int waterLevel)
{
  /*
  for(int i=0; i < 16; ++i)
  {
    for(int j=0; j < 16; ++j)
    {
      if(!used.Info[i][j])continue;
      if(Info[i][j][0].Flags==2)Info[i][j][0].Flags=0;
      MH2O_Tile lTile = Liquids[i][j][0]->getMH2OData();
      used.HeightData[i][j]=true;
      used.TransparencyData[i][j]=true;
      for(int h=0 ; h < 9; ++h){
        for(int w=0; w < 9; ++w){
          lTile.mHeightmap[w][h]+=waterLevel;//visual change

          used.HeightDataPr[i][j][w][h]=true;
          if(used.HeightDataPr[i][j][w][h]){
            HeightData[i][j][0].mHeightValues[w][h]+=waterLevel;
            if(Info[i][j][0].maxHeight<HeightData[i][j][0].mHeightValues[w][h])
              Info[i][j][0].maxHeight=HeightData[i][j][0].mHeightValues[w][h];
            if(Info[i][j][0].minHeight>HeightData[i][j][0].mHeightValues[w][h])
              Info[i][j][0].minHeight=HeightData[i][j][0].mHeightValues[w][h];
          }
          used.TransparencyDataPr[i][j][w][h]=true;
        }
      }

      Liquids[i][j][0]->setMH2OData(lTile);
      Liquids[i][j][0]->initFromMH2O();
    }
  }
  */
}

int TileWater::getLevel()
{
  //TODO: Beket implement water typ
  return false;
}

void TileWater::setOpercity(int opercity)
{
  //TODO: Beket implement water opercity
}

int TileWater::getOpercity()
{
  //TODO: Beket implement water opercity
  return false;
}

void TileWater::setType(int type)
{
  //TODO: Beket implement water type
}

int TileWater::getType()
{
  //TODO: Beket implement water type
  return false;
}



