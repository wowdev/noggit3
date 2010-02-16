/*
 * MCIN.cpp
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */

#include "MCIN.h"

namespace libworld
{
namespace adtlib
{

MCIN::MCIN(){
	Buffer b=Buffer(data+256*sizeof(MCIN_Entry));
	this->buff=b;
	this->buff.setPosition(magic);
	char m[]={'N','I','C','M'};
	Magic=MagicToInt(m);
}

MCIN::MCIN(Buffer buff){
		_Tn("MCIN::MCIN(Buffer buff)");
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		this->buff.setPosition(data);
		for(int i=0;i<256;i++){
			this->buff >>Info[i];
		}
}

void MCIN::Render(){
	ChunkSize=256*sizeof(MCIN_Entry);
	buff=Buffer(data+ChunkSize);
	buff<<Magic;
	buff<<ChunkSize;
	this->buff.setPosition(data);
	for(int i=0;i<256;i++){
		this->buff <<Info[i];
	}
}

Buffer MCIN::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

MCIN_Entry MCIN::getMCNKInfo(int n){
	return Info[n];
}

MCIN_Entry MCIN::getMCNKInfo(int x,int y){
	return Info[x*16+y];
}

MCIN::~MCIN(){

}

}
}