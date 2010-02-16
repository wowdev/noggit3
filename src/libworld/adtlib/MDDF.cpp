/*
 * MDDF.cpp
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#include "MDDF.h"

namespace libworld
{
namespace adtlib
{

MDDF::MDDF(){
	buff=Buffer(data);
	char m[]={'F','D','D','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MDDF::MDDF(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff>>Magic>>ChunkSize;
	nEntries=ChunkSize/sizeof(MDDF_Entry);
	entries=new MDDF_Entry[nEntries];
	for(int i=0;i<nEntries;i++){
		this->buff>>entries[i];
	}
}

void MDDF::Render(){
	ChunkSize=nEntries*sizeof(MDDF_Entry);
	buff=Buffer(data+ChunkSize);
	buff.setPosition(magic);
	buff<<Magic;
	buff<<ChunkSize;
	buff.setPosition(data);
	for(int i=0;i<nEntries;i++){
		this->buff<<entries[i];
	}
}

Buffer MDDF::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

MDDF::~MDDF(){

}

}
}