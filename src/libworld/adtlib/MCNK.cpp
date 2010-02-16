/*
 * MCNK.cpp
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */

#include "MCNK.h"

namespace libworld
{
namespace adtlib
{

MCNK::MCNK(){
	Buffer b=Buffer(data+sizeof(MCNK_Header));
	this->buff=b;
	this->buff.setPosition(magic);
	char m[]={'K','N','C','M'};
	Magic=MagicToInt(m);
}

MCNK::MCNK(Buffer buff){
		_Tn("MCNK::MCNK(Buffer buff)");
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		this->buff.setPosition(data);
		this->buff>>info;

		mcvt=MCVT(Buffer(this->buff,info.ofsMCVT));
		mcly=MCLY(Buffer(this->buff,info.ofsMCLY));
}

void MCNK::Render(){

}

Buffer MCNK::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}


void MCNK::removeMCSE(){
	_TDn("void MCNK::removeMCSE()");
	//0x58==ofsSndEmitters 0x5C==nSndEmitters
	int n=0;
	this->buff.setPosition(0x58+data);
	this->buff<<n<<n;
}

MCNK::~MCNK(){

}

}
}