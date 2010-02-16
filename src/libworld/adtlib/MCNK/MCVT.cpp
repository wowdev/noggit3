/*
 * MCVT.cpp
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */
#include "MCVT.h"

namespace libworld
{
namespace adtlib
{

MCVT::MCVT(Buffer buff){
		_Tn("buff-Konstruktor");
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		this->buff.setPosition(data);
		for(int i=0;i<145;i++){
			this->buff>>heightvalues[i];
		}
}

MCVT::MCVT(){
	buff=Buffer(data+145*sizeof(int));
	char m[]={'T','V','C','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

void MCVT::Render(){
	ChunkSize=145*sizeof(float);
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic;
	this->buff<<ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<145;i++){
		this->buff<<heightvalues[i];
	}
}

Buffer MCVT::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}


float MCVT::getValLOD(int x,int y){
	int add = (y+1)*9;
	return heightvalues[y*8 + x + add];
}

float MCVT::getValNoLOD(int x,int y){
	int add = y*8;
	return heightvalues[y*9 + x + add];
}

float MCVT::getVal(int index){
	return heightvalues[index];
}

void MCVT::setValLOD(float val,int x,int y){
	int add = (y+1)*9;
	heightvalues[y*8 + x + add]=val;
}

void MCVT::setValNoLOD(float val,int x,int y){
	int add = y*8;
	heightvalues[y*9 + x + add]=val;
}

void MCVT::setVal(float val,int index){
	heightvalues[index]=val;
}

MCVT::~MCVT(){

}

}
}