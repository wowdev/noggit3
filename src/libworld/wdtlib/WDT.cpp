/*
 * WDT.cpp
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#include "WDT.h"

namespace libworld
{
namespace wdtlib
{

WDT::WDT(){
	mver=MVER();
	mphd=MPHD();
	main=MAIN();
	mwmo=libworld::adtlib::MWMO();
}

WDT::WDT(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(0);
	int ofs=0;
	mver=MVER(this->buff);
	ofs+=mver.getRealSize();
	Buffer tBuff=Buffer(this->buff,ofs);
	mphd=MPHD(tBuff);
	ofs+=mphd.getRealSize();
	tBuff=Buffer(this->buff,ofs);
	main=MAIN(tBuff);
	ofs+=main.getRealSize();
	tBuff=Buffer(this->buff,ofs);
	//should always be true at this point...
	if(tBuff.hasRemaining()){
		mwmo=libworld::adtlib::MWMO(tBuff);
		ofs+=mwmo.getRealSize();
		if((this->buff.getSize()<ofs)&&!mphd.hasTerrain())
			tBuff=Buffer(this->buff,ofs);
			modf=libworld::adtlib::MODF(tBuff);
	}

}

void WDT::Render(){
	this->buff=mver.getChunk();
	buff+=mphd.getChunk();
	buff+=main.getChunk();
	buff+=mwmo.getChunk();
	if(!hasTerrain())buff+=modf.getChunk();
}

Buffer WDT::getFileBuff(){
	Render();
	this->buff.setPosition(0);
	return this->buff;
}

bool WDT::hasTerrain(){
	return mphd.hasTerrain();
}

bool WDT::hasBigAlpha(){
	return mphd.hasBigAlpha();
}

void WDT::unsetTerrain(){
	mphd.unsetTerrain();
	modf=libworld::adtlib::MODF();
}

void WDT::setTerrain(){
	mphd.setTerrain();
	mwmo=libworld::adtlib::MWMO();
}

bool WDT::hasADT(int i){
	return main.hasADT(i);
}

bool WDT::hasADT(int x,int y){
	return main.hasADT(x*64+y);
}

void WDT::setADT(int i){
	main.setADT(i);
}

void WDT::setADT(int x,int y){
	main.setADT(x*64+y);
}

void WDT::unsetADT(int i){
	main.unsetADT(i);
}
void WDT::unsetADT(int x,int y){
	main.unsetADT(x*64+y);
}

/*
*There is max. one WMO globally spawned!
*/
char *WDT::getWMOName(){
	return mwmo.getObjectName(0);
}
int WDT::getLengthWMOName(){
	return mwmo.getlenObjectName(0);
}

libworld::adtlib::MODF_Entry WDT::getWMOInfo(){
	return modf.getEntry(0);
}

WDT::~WDT(){

}

}
}