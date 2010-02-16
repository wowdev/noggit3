/*
 * MAOF.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#include "MAOF.h"

namespace libworld
{
namespace wdllib
{

MAOF::MAOF(){
	_TDn("MAOF::MAOF()");
	buff=Buffer(data+sizeof(int)*4096);
	char m[]={'F','O','A','M'};
	Magic=MagicToInt(m);
	ChunkSize=sizeof(int)*4096;
	for(int i=0;i<4096;i++){
		aOffset[i]=0;
	}
}

MAOF::MAOF(Buffer buff){
	_TDn("MAOF::MAOF(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic;
	printf("%d m\n",Magic);
	this->buff >> ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<4096;i++){
		this->buff>>aOffset[i];
	}
}

void MAOF::Render(){
	_TDn("void MAOF::Render()");
	ChunkSize=4096*sizeof(int);
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic;
	this->buff<<ChunkSize;
	this->buff.setPosition(0x4);
	this->buff.setPosition(data);
	for(int i=0;i<4096;i++){
		this->buff<<aOffset[i];
	}
}

Buffer MAOF::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MAOF::getOffset(int index){
	_TD("int MAOF::getOffset(index: ","%d)",index);
	return aOffset[index];
}
int MAOF::getOffset(int x,int y){
	_TD("int MAOF::getOffset(x: ","%d,y: %d",x,y);
	int index = (x*64) + y;
	return aOffset[index];
}

void MAOF::setOffset(int val,int index){
	_TD("void MAOF::setOffset(val: ","%d,index: %d",val,index);
	aOffset[index]=val;
}
void MAOF::setOffset(int val,int x,int y){
	_TD("void MAOF::setOffset(val: ","%d,x: %d,y: %d",val,x,y);
	int index = (x*64) + y;
	aOffset[index]=val;
}

MAOF::~MAOF(){
	_TDn("MAOF::~MAOF()");

}

}
}