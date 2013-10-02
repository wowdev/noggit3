#include "TileWater.h"

#include "Log.h"
#include "Misc.h"
#include "ChunkWater.h"

TileWater::TileWater(float pXbase, float pZbase)
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

void TileWater::saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition){
  /*
  int waterSize = 0; //used water size. Needed for mh2o header.
  int ofsW = lCurrentPosition + 0x8; //water Header pos

  lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14; //setting offset to MH2O data in Header
  lADTFile.Extend(8); // we need 8 empty bytes
  lCurrentPosition = ofsW;

  //writing MH2O_Header
  for(int i=0; i < 16; ++i) {
    for(int j=0; j < 16; ++j) {
      if(!used.Header[i][j])continue; //header not used

      lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Header), reinterpret_cast<char*>(&Header[i][j]));
      waterSize += sizeof(MH2O_Header);
      lCurrentPosition += sizeof(MH2O_Header);
    }
  }

  //writing MH2O_Information
  for(int i=0; i < 16; ++i) {
    for(int j=0; j < 16; ++j) {
      if(!used.Info[i][j])continue; //info not used

      lADTFile.Insert(lCurrentPosition, sizeof(MH2O_Information), reinterpret_cast<char*>(&Info[i][j][0])); //insert MH2O_Information
      lADTFile.GetPointer<MH2O_Header>(ofsW + (16 * i + j) * sizeof(MH2O_Header))->ofsInformation = lCurrentPosition - ofsW; //setting offset to this info at the header
      lADTFile.GetPointer<MH2O_Header>(ofsW + (16 * i + j) * sizeof(MH2O_Header))->nLayers = 1; //setting number of layers
      lCurrentPosition += sizeof(MH2O_Information);
      waterSize += sizeof(MH2O_Information);
    }
  }

  //writing other Info
  for(int i=0; i < 16; ++i) {
    for(int j=0; j < 16; ++j) {

      //render
      if(used.Render[i][j]){
        char wRender[8];
        char fRender[8];
        for(int h=0 ; h < 8; ++h) {
          for(int w=0; w < 8; ++w) {
            Render[i][j][0].mRender[w][h]? (wRender[w] |= (1<<h)) : (wRender[w] &=~(1<<h)); //render mask
            Render[i][j][0].fRender[w][h]? (fRender[w] |= (1<<h)) : (fRender[w] &=~(1<<h)); //fatigue render mask?
          }
        }
        lADTFile.Insert(lCurrentPosition, sizeof(wRender), reinterpret_cast<char*>(&wRender));
        lADTFile.Insert(lCurrentPosition+8, sizeof(fRender), reinterpret_cast<char*>(&fRender));
        lADTFile.GetPointer<MH2O_Header>(ofsW + (16 * i + j) * sizeof(MH2O_Header))->ofsRenderMask = lCurrentPosition - ofsW;
        waterSize += 2*sizeof(wRender);
        lCurrentPosition +=  2*sizeof(wRender);
      }

      //mask
      if(used.Mask[i][j]){
        int maskLen = Info[i][j][0].height;
        lADTFile.Insert(lCurrentPosition, maskLen, reinterpret_cast<char*>(&Mask[i][j][0]));
        lADTFile.GetPointer<MH2O_Information>(ofsW + lADTFile.GetPointer<MH2O_Header>(ofsW + (16 * i + j) * sizeof(MH2O_Header))->ofsInformation)->ofsInfoMask = lCurrentPosition - ofsW;
        waterSize += maskLen;
        lCurrentPosition +=  maskLen;
      }

      //setting offset to HeightData/Transparency
      if(used.HeightData[i][j] || used.TransparencyData[i][j]){
        lADTFile.GetPointer<MH2O_Information>(ofsW + lADTFile.GetPointer<MH2O_Header>(ofsW + (16 * i + j) * sizeof(MH2O_Header))->ofsInformation)->ofsHeightMap = lCurrentPosition - ofsW;
      }

      //HeighData
      if(used.HeightData[i][j]){
        for (int w = Info[i][j][0].yOffset; w < Info[i][j][0].yOffset + Info[i][j][0].height + 1; ++w) {
          for(int h=Info[i][j][0].xOffset; h < Info[i][j][0].xOffset + Info[i][j][0].width + 1; ++h) {
            lADTFile.Insert(lCurrentPosition, sizeof(float), reinterpret_cast<char*>(&HeightData[i][j][0].mHeightValues[w][h]));
            lCurrentPosition += sizeof(float);
            waterSize += sizeof(float);
          }
        }
      }

      //TransparencyData

      if(used.TransparencyData[i][j]){
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

        for (int w = Info[i][j][0].yOffset; w < Info[i][j][0].yOffset + Info[i][j][0].height + 1; ++w) {
          for(int h=Info[i][j][0].xOffset; h < Info[i][j][0].xOffset + Info[i][j][0].width + 1; ++h) {
            lADTFile.Insert(lCurrentPosition, sizeof(unsigned char), reinterpret_cast<char*>(&HeightData[i][j][0].mTransparency[w][h]));
            lCurrentPosition+=sizeof(unsigned char);
            waterSize += sizeof(unsigned char);
          }
        }
      }
    }
  }

  SetChunkHeader( lADTFile, ofsW - 8, 'MH2O', waterSize);
  lCurrentPosition += 8;
  */

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

void TileWater::setOpercity( int opercity )
{
  //TODO: Beket implement water opercity
}

int TileWater::getOpercity()
{
  //TODO: Beket implement water opercity
  return false;
}

void TileWater::setType( int typ )
{
  //TODO: Beket implement water typ
}

int TileWater::getType()
{
  //TODO: Beket implement water typ
  return false;
}



