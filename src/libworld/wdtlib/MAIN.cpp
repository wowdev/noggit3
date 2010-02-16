/*
 * MAIN.cpp
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */
#include "MAIN.h"

namespace libworld
{
namespace wdtlib
{

MAIN::MAIN(){
	int Size=4096*sizeof(Main_Object)+data;
	Buffer b=Buffer(Size);
	this->buff=b;
	this->buff.setPosition(magic);
	this->ChunkSize=0x8000;
	char m[]={'N','I','A','M'};
	Magic=MagicToInt(m);
}

MAIN::MAIN(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize;
	for(int i=0;i<64*64;i++)
		this->buff>>table[i];
}

void MAIN::Render(){
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic<<ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<64*64;i++)
		this->buff<<table[i];
}

Buffer MAIN::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

bool MAIN::hasADT(int i){
	return (table[i].flags&1);
}

bool MAIN::hasADT(int x,int y){
	return hasADT(x*64+y);
}

void MAIN::setADT(int i){
	if(!hasADT(i))table[i].flags+=1;
}

void MAIN::setADT(int x,int y){
	setADT(x*64+y);
}

void MAIN::unsetADT(int i){
	table[i].flags=((table[i].flags&1)==1)
				? table[i].flags-1:table[i].flags;
}
void MAIN::unsetADT(int x,int y){
	unsetADT(x*64+y);
}

MAIN::~MAIN(){

}

}
}
