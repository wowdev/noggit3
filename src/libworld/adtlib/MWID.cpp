/*
 * MWID.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#include "MWID.h"

namespace libworld
{
namespace adtlib
{

MWID::MWID(){
	buff=Buffer(data);
	char m[]={'D','I','W','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MWID::MWID(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff>>Magic>>ChunkSize;
	nOffsets=ChunkSize/sizeof(int);
	ObjectOffsets=new int[nOffsets];
	this->buff.setPosition(data);
	for(int i=0;i<nOffsets;i++){
		this->buff>>ObjectOffsets[i];
	}
}

void MWID::Render(){
	ChunkSize=nOffsets*sizeof(int);
	buff=Buffer(data+ChunkSize);
	this->buff<<Magic<<ChunkSize;
	for(int i=0;i<nOffsets;i++){
		this->buff<<ObjectOffsets[i];
	}
}

Buffer MWID::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MWID::getOffset(int i){
	return ObjectOffsets[i];
}

void MWID::setOffset(int val,int i){
	ObjectOffsets[i]=val;
}

int MWID::getnOffsets(){
	return nOffsets;
}
/*
 * Most likely this is only usable for some hacking :P
 */
void MWID::setnOffsets(int n){
	nOffsets=n;
}

void MWID::addOffset(int n){
	int *t=ObjectOffsets;
	ObjectOffsets=new int[nOffsets+1];
	memcpy(ObjectOffsets,t,sizeof(int)*nOffsets);
	ObjectOffsets[nOffsets]=n;
	++nOffsets;
}

MWID::~MWID(){

}

}
}