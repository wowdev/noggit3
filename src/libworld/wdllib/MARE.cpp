/*
 * MARE.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */
#include "MARE.h"

namespace libworld
{
namespace wdllib
{

MARE::MARE(){
	//_Tn("MARE::MARE()");
	buff=Buffer(data+sizeof(short)*16*16*17*17);
	char m[]={'E','R','A','M'};
	Magic=MagicToInt(m);
	ChunkSize=sizeof(short)*16*16*17*17;
	for(int i=0;i<16*16;i++){
		InnerHeight[i]=0;
	}
	for(int i=0;i<17*17;i++){
		OuterHeight[i]=0;
	}
}

MARE::MARE(Buffer buff){
	_TDn("MARE::MARE(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<17*17;i++){
		this->buff>>OuterHeight[i];
	}
	for(int i=0;i<16*16;i++){
		this->buff>>InnerHeight[i];
	}
}

void MARE::Render(){
	_TDn("void MARE::Render()");
	ChunkSize=sizeof(short)*16*16*17*17;
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic<<ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<17*17;i++){
		this->buff<<OuterHeight[i];
	}
	for(int i=0;i<16*16;i++){
		this->buff<<InnerHeight[i];
	}
}

short MARE::getInnerHeight(int index){
	_TD("short MARE::getInnerHeight(index: ","%d)",index);
	return InnerHeight[index];
}
short MARE::getInnerHeight(int x,int y){
	int index = (x*16) + y;
	return InnerHeight[index];
}

void MARE::setInnerHeight(short val,int index){
	InnerHeight[index]=val;
}
void MARE::setInnerHeight(short val,int x,int y){
	int index = (x*16) + y;
	InnerHeight[index]=val;
}


short MARE::getOuterHeight(int index){
	return OuterHeight[index];
}
short MARE::getOuterHeight(int x,int y){
	int index = (x*17) + y;
	return OuterHeight[index];
}

void MARE::setOuterHeight(short val,int index){
	OuterHeight[index]=val;
}
void MARE::setOuterHeight(short val,int x,int y){
	int index = (x*17) + y;
	OuterHeight[index]=val;
}





MARE::~MARE(){
	//_Tn("MARE::~MARE()");

}

}
}