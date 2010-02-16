/*
 * Chunk.cpp
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#include "Chunk.h"

namespace libworld
{

Chunk::Chunk(){
	//_Tn("Chunk::Chunk()");
	Buffer b=Buffer(data);
	this->buff=b;
	this->buff.setPosition(magic);
}

Chunk::Chunk(Buffer buff){
	//_Tn("Chunk::Chunk(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize;
}


int Chunk::getMagic(){
	_TDn("char *Chunk::getMagic()");
	return Magic;
}

void Chunk::setMagic(int m){
	_T("void Chunk::setMagic","(%s)",m);
	this->buff.put<int>(m,magic);
	this->Magic=m;
}

void Chunk::setChunkSize(int Size){
	_T("void Chunk::setChunkSize","(%d)",Size);
	this->buff.put<int>(Size,chunksize);
	this->ChunkSize=Size;
}

int Chunk::getChunkSize(){
	_TDn("int Chunk::getChunkSize()");
	return ChunkSize;
}

int Chunk::getRealSize(){
	_TDn("int Chunk::getRealSize()");
	return this->ChunkSize+data;
}

void Chunk::Render(){
	this->buff=Buffer(data+ChunkSize);
	this->buff<<Magic<<ChunkSize;
}

Buffer Chunk::getChunk(){
	Render();
	this->buff.setPosition(0);
	return this->buff;
}

bool Chunk::MagicCorrect(){
	return true;
}

int Chunk::MagicToInt(char magic[4]){
	int ret=0;
	ret+=magic[0];
	ret+=magic[1]<<8;
	ret+=magic[2]<<16;
	ret+=magic[3]<<24;
	return ret;
}


Chunk::~Chunk(){

}

}