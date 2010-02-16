/*
 * MPHD.cpp
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#include "MPHD.h"

namespace libworld
{
namespace wdtlib
{

MPHD::MPHD(){
	int Size=8*sizeof(int)+data;
	Buffer b=Buffer(Size);
	this->buff=b;
	this->buff.setPosition(magic);
	this->ChunkSize=0x20;
	char m[]={'D','H','P','M'};
	Magic=MagicToInt(m);
}

MPHD::MPHD(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize>> flags>>something;
	for(int i=0;i<6;i++)
		this->buff>>unused[i];
}

void MPHD::Render(){
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic<<ChunkSize<<flags<<something;
	for(int i=0;i<6;i++)
		this->buff<<unused[i];
}

Buffer MPHD::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

bool MPHD::hasTerrain(){
	return !(flags&Map_No_Terrain);
}

bool MPHD::hasBigAlpha(){
	return (flags&Map_TerrainShaders_BigAlpha);
}


void MPHD::unsetTerrain(){
	if(hasTerrain())flags+=Map_No_Terrain;
}

void MPHD::setTerrain(){
	flags=((flags&Map_No_Terrain)==Map_No_Terrain)
				? flags-Map_No_Terrain:flags;
}
MPHD::~MPHD(){

}

}
}