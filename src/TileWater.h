#ifndef TILEWATER_H
#define TILEWATER_H

#include "MPQ.h"
#include "MapHeaders.h"
#include "MapTile.h"
#include "Vec3D.h"
#include "Liquid.h"

class Liquid;
class sExtendableArray;

class TileWater
{

public:
	TileWater(bool waterExists);
	MH2O_Header Header[16][16];
	MH2O_Information Info[16][16][5];
	char *Mask[16][16][5];
	MH2O_HeightMask HeightData[16][16][5];
	MH2O_Render Render[16][16][5];

	MH2O_UsedChunks used; //registry of used data

	Liquid * Liquids[16][16][5];

	void readFromFile(MPQFile &theFile, int &ofsW);
	void saveToFile(sExtendableArray &lADTFile, int &lMHDR_Position, int &lCurrentPosition);
	void init(float xbase, float zbase);
	void draw();
	void setWaterLevel(int waterLevel);
	
	virtual ~TileWater(void);
private:
	bool hasWater;
};

#endif