/*
 * MVER.cpp
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#include "MVER.h"

namespace libworld
{

MVER::MVER(){
	int Size=12;
	char *f=new char[12];
	Buffer b=Buffer(f,Size);
	this->buff=b;
	this->buff.setPosition(magic);
	this->ChunkSize=4;
	Version=0x12;
	char m[]={'R','E','V','M'};
	Magic=MagicToInt(m);

}

MVER::MVER(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize>> Version;

}


int MVER::getVersion(){
	return Version;
}

void MVER::setVersion(int vers){
	this->Version=vers;
}

void MVER::Render(){
	ChunkSize=4;
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic;
	this->buff<<ChunkSize;
	this->buff<<Version;
}

Buffer MVER::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}


MVER::~MVER(){

}

}