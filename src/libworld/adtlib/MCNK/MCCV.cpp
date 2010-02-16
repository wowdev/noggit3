/*
 * MCVT.cpp
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */
#include "MCCV.h"

namespace libworld
{
namespace adtlib
{

MCCV::MCCV(Buffer buff){
		_Tn("buff-Konstruktor");
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		this->buff.setPosition(data);
		for(int i=0;i<145;i++){
			this->buff>>colorvalues[i];
		}
}

MCCV::MCCV(){
	buff=Buffer(data+145*sizeof(int));
	char m[]={'V','C','C','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

void MCCV::Render(){
	ChunkSize=145*sizeof(int);
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic;
	this->buff<<ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<145;i++){
		this->buff<<colorvalues[i];
	}
}

Buffer MCCV::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MCCV::getValLOD(int x,int y){
	int add = (y+1)*9;
	return colorvalues[y*8 + x + add];
}

void MCCV::setValLOD(int val,int x,int y){
	int add = (y+1)*9;
	colorvalues[y*8 + x + add]=val;
}

int MCCV::getValNoLOD(int x,int y){
	int add = y*8;
	return colorvalues[y*9 + x + add];
}

void MCCV::setValNoLOD(int val,int x,int y){
	int add = y*8;
	colorvalues[y*9 + x + add]=val;
}

int MCCV::getVal(int index){
	return colorvalues[index];
}

void MCCV::setVal(int val,int index){
	colorvalues[index]=val;
}

MCCV::~MCCV(){

}

}
}