/*
 * MCLY.cpp
 *
 *  Created on: 09.08.2009
 *      Author: Bastian
 */

#include "MCLY.h"

namespace libworld
{
namespace adtlib
{

MCLY::MCLY(){
	buff=Buffer(data);
	char m[]={'Y','L','C','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MCLY::MCLY(Buffer buff){
	_Tn("MCLY::MCLY(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize;
	this->buff.setPosition(data);
	nEntries=ChunkSize/sizeof(MCLY_Entry);
	Entries=new MCLY_Entry[nEntries];
	for(int i=0;i<nEntries;i++){
		this->buff>>Entries[i];
	}
}


MCLY::~MCLY(){

}

}
}