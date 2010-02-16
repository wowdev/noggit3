/*
 * MWID.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#include "MMID.h"

namespace libworld
{
namespace adtlib
{

MMID::MMID(){
	buff=Buffer(data);
	char m[]={'D','I','M','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MMID::MMID(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff>>Magic>>ChunkSize;
	nOffsets=ChunkSize/sizeof(int);
	DoodadOffsets=new int[nOffsets];
	this->buff.setPosition(data);
	for(int i=0;i<nOffsets;i++){
		this->buff>>DoodadOffsets[i];
	}
}

void MMID::Render(){
	ChunkSize=nOffsets*sizeof(int);
	buff=Buffer(data+ChunkSize);
	this->buff<<Magic<<ChunkSize;
	for(int i=0;i<nOffsets;i++){
		this->buff<<DoodadOffsets[i];
	}
}

Buffer MMID::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MMID::getOffset(int i){
	return DoodadOffsets[i];
}

void MMID::setOffset(int val,int i){
	DoodadOffsets[i]=val;
}

int MMID::getnOffsets(){
	return nOffsets;
}
/*
 * Most likely this is only usable for some hacking :P
 */
void MMID::setnOffsets(int n){
	nOffsets=n;
}

void MMID::addOffset(int n){
	int *t=DoodadOffsets;
	DoodadOffsets=new int[nOffsets+1];
	memcpy(DoodadOffsets,t,sizeof(int)*nOffsets);
	DoodadOffsets[nOffsets]=n;
	++nOffsets;
}

MMID::~MMID(){

}

}
}