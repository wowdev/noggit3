#include "TileWater.h"

#include "Log.h"
#include "Misc.h"


TileWater::TileWater(bool waterExists){
	this->hasWater = waterExists;
}

void TileWater::readFromFile(MPQFile &theFile, int &ofsW){
	used = MH2O_UsedChunks(); //preparing our registry of used items

	//MH2O_Header reading
	for(int i=0; i < 16; ++i) {
      for(int j=0; j < 16; ++j) {
		theFile.seek(ofsW + (i * 16 + j) * sizeof(MH2O_Header));
		theFile.read(&Header[i][j], sizeof(MH2O_Header));
		used.Header[i][j] = true;
      }
    }

    //water info reading
	for(int i=0; i < 16; ++i) {
      for(int j=0; j < 16; ++j) {
		if(Header[i][j].nLayers > 0){
		  for(int k=0; k < Header[i][j].nLayers; ++k){
		    if(k > 0)break; //Temporary only for 1 layer

			//info
			theFile.seek(ofsW + Header[i][j].ofsInformation);
			theFile.read(&Info[i][j][k], sizeof(MH2O_Information));
			used.Info[i][j] = true;

			//render
			if(Header[i][j].ofsRenderMask != 0 ) {
				byte rMask[8];
				byte fMask[8];
				theFile.seek(ofsW + Header[i][j].ofsRenderMask);
				theFile.read(rMask,sizeof(rMask));
				theFile.read(fMask,sizeof(fMask));
				for(int h=0 ; h < 8; ++h) {
					for(int w=0; w < 8; ++w) {
						Render[i][j][k].mRender[w][h] = (1 << h) & rMask[w];
						Render[i][j][k].fRender[w][h] = (1 << h) & fMask[w];
						used.Render[i][j] = true;
					}
				}
			} else {	//if we have no MH2O_Render structure
				for(int h=0 ; h < 8; ++h) {
					for(int w=0; w < 8; ++w) {
						Render[i][j][k].mRender[w][h] = true;
						Render[i][j][k].fRender[w][h] = true;
					}
				}
			}

			//mask
			if(Info[i][j][k].ofsInfoMask > 0){
				int numBytes = Info[i][j][k].height;//(Info[i][j][k].width * Info[i][j][k].height) / 8;
			    theFile.seek(Info[i][j][k].ofsInfoMask + ofsW);
				/*if(numBytes == 0 && (Info[i][j][k].width > 0 && Info[i][j][k].height > 0))
					numBytes = 1;*/
			    if(numBytes>0){
					Mask[i][j][k] = new char[numBytes];
					theFile.read(&Mask[i][j][k], numBytes);
					used.Mask[i][j] = true;
			    }
			}
			

			//height
			HeightData[i][j][k] = MH2O_HeightMask(9,9); //standart mask 9*9 with nulls
			if(Info[i][j][k].ofsHeightMap != 0){// && !(Info[i][j][k].Flags & 2)) {
			  theFile.seek(ofsW + Info[i][j][k].ofsHeightMap);

				if(Info[i][j][k].Flags != 2){ //if Flags == 2 then there are no Heighmap data...
					used.HeightData[i][j] = true;
					for (int w = Info[i][j][k].yOffset; w < Info[i][j][k].yOffset + Info[i][j][k].height + 1; ++w) {
						for(int h=Info[i][j][k].xOffset; h < Info[i][j][k].xOffset + Info[i][j][k].width + 1; ++h) {
							theFile.read(&HeightData[i][j][k].mHeightValues[w][h], sizeof(float));
							used.HeightDataPr[i][j][w][h] = true;
						}
					}		
				}

				used.TransparencyData[i][j] = true;
				for (int w = Info[i][j][k].yOffset; w < Info[i][j][k].yOffset + Info[i][j][k].height + 1; ++w) {
					for(int h=Info[i][j][k].xOffset; h < Info[i][j][k].xOffset + Info[i][j][k].width + 1; ++h) {
						theFile.read(&HeightData[i][j][k].mTransparency[w][h], sizeof(unsigned char));
						used.TransparencyDataPr[i][j][w][h] = true;
					}
				}
            }
		  }
		} else {
		  Info[i][j][0] = MH2O_Information(); //chunk has no water
		}  
	  }
	}
}

void TileWater::init(float xbase, float zbase){
  for(int i=0; i < 16; ++i) {
      for(int j=0; j < 16; ++j) {
		
		Liquids[i][j][0] = new Liquid(Info[i][j][0].width, Info[i][j][0].height, Vec3D( xbase + CHUNKSIZE * j, Info[i][j][0].minHeight, zbase + CHUNKSIZE * i ) );

		MH2O_Tile lTile;
        lTile.mLiquidType = Info[i][j][0].LiquidType;
        lTile.mMaximum = Info[i][j][0].maxHeight;
        lTile.mMinimum = Info[i][j][0].minHeight;
        lTile.mFlags = Info[i][j][0].Flags;
		
        for( int x = 0; x < 9; ++x ) {
          for( int y = 0; y < 9; ++y ) {
			  lTile.mHeightmap[x][y] = (used.HeightDataPr[i][j][x][y]&&HeightData[i][j][0].mHeightValues[x][y]>0)? (HeightData[i][j][0].mHeightValues[x][y]) : lTile.mMinimum;
			  lTile.mDepth[x][y] = used.TransparencyDataPr[i][j][x][y]? (HeightData[i][j][0].mTransparency[x][y] / 255.0f) : 1.0f;
          }
        }
        
		for(int h=0 ; h < 8; ++h) {
			for(int w=0; w < 8; ++w) {
				lTile.mRender[w][h] = (used.HeightDataPr[i][j][w][h]&&HeightData[i][j][0].mHeightValues[w][h]>0)?HeightData[i][j][0].mHeightValues[w][h] : Render[i][j][0].mRender[w][h];
			}
		}
		Liquids[i][j][0]->setMH2OData(lTile);
	  }
  }
}

void TileWater::draw(){
	for(int i=0; i < 16; ++i) {
		for(int j=0; j < 16; ++j) {
			if(used.Info[i][j]){	
				Liquids[i][j][0]->draw();
			}
		}
	}
}

void TileWater::saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition){
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
			byte wRender[8];
			byte fRender[8];
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
			/*bool eFlag = true; //Blizz zips their files here using same bytes at mask and transparency. They move transparency bytes back if some last bytes of previous data matches some first bytes of next data.
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

			lCurrentPosition-=offs-1;*/

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

}

TileWater::~TileWater(void){

}

// following functions change the full ADT
void TileWater::setLevel(int waterLevel){
	if(!hasWater)return;
	for(int i=0; i < 16; ++i) {
		for(int j=0; j < 16; ++j) {
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



