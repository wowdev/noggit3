/*
 * MAHO.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#include "MAHO.h"

namespace libworld
{
namespace wdllib
{

MAHO::MAHO(){
	//_Tn("MAHO::MAHO()");
	buff=Buffer(data+sizeof(short)*16);
	char m[]={'O','H','A','M'};
	Magic=MagicToInt(m);
	ChunkSize=sizeof(short)*16;
	for(int i=0;i<16;i++){
		mask[i]=0;
	}
}

MAHO::MAHO(Buffer buff){
	_TDn("MAHO::MAHO(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<16;i++){
		this->buff>>mask[i];
	}
}

bool MAHO::MagicCorrect(){
	char m[]={'O','H','A','M'};
	if(MagicToInt(m)!=Magic)
		return false;
	return true;
}

void MAHO::Render(){
	_TDn("void MAHO::Render()");
	ChunkSize=sizeof(short)*16;
	this->buff=Buffer(data+ChunkSize);
	this->buff<<Magic<<ChunkSize;
	for(int i=0;i<16;i++){
		this->buff<<mask[i];
	}
}

void MAHO::setMask(short val,int n){
	_TD("void MAHO::setMask(val: ","%d, n: %d)",val,n);
	mask[n]=val;
}

short MAHO::getMask(int n){
	_TD("short MAHO::getMask(n: ","%d)",n);
	return mask[n];
}

MAHO::~MAHO(){
	_Tn("MAHO::~MAHO()");

}

}
}