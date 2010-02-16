/*
 * WoWFile.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#include "WoWFile.h"

namespace libworld
{

WoWFile::WoWFile(){
	_TDn("WoWFile::WoWFile()");

}

WoWFile::WoWFile(Buffer buff){
	_TDn("WoWFile::WoWFile(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(0);
	mver=MVER(this->buff);
}

Buffer WoWFile::getFileBuff(){
	Render();
	this->buff.setPosition(0);
	return this->buff;
}

void WoWFile::Render(){
	_TDn("void WoWFile::Render()");
	buff=Buffer();
	buff+=mver.getChunk();

}

WoWFile::~WoWFile(){
	_TDn("WoWFile::~WoWFile()");

}

}