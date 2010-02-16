/*
 * ADT.cpp
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#include "ADT.h"

namespace libworld
{
namespace adtlib
{

ADT::ADT(){
	mver=MVER();
	mhdr=MHDR();
	mcin=MCIN();
	mtex=MTEX();
	mmdx=MMDX();
	mmid=MMID();
	mwmo=MWMO();
	mwid=MWID();
	mddf=MDDF();
	modf=MODF();

}

ADT::ADT(Buffer buff){
	this->buff=buff;
	this->buff.setPosition(0);
	mver=MVER(this->buff);
	mhdr=MHDR(Buffer(this->buff,mver.getRealSize()));
	mcin=MCIN(Buffer(this->buff,mhdr.getOffsetMCIN()));
	mtex=MTEX(Buffer(this->buff,mhdr.getOffsetMTEX()));
	mmdx=MMDX(Buffer(this->buff,mhdr.getOffsetMMDX()));
	mmid=MMID(Buffer(this->buff,mhdr.getOffsetMMID()));
	mwmo=MWMO(Buffer(this->buff,mhdr.getOffsetMWMO()));
	mwid=MWID(Buffer(this->buff,mhdr.getOffsetMWID()));
	mddf=MDDF(Buffer(this->buff,mhdr.getOffsetMDDF()));
	modf=MODF(Buffer(this->buff,mhdr.getOffsetMODF()));


	/*for(int i=0;i<256;i++){
		mcnk[i]=MCNK(Buffer(this->buff,mcin.getMCNKInfo(i).ofsMCNK));
	}*/
}

void ADT::Render(){
	buff=Buffer();
	buff=mver.getChunk();

	buff+=mhdr.getChunk();

	mhdr.setOffsetMCIN(buff.getSize());
	buff+=mcin.getChunk();

	mhdr.setOffsetMTEX(buff.getSize());
	buff+=mtex.getChunk();

	mhdr.setOffsetMMDX(buff.getSize());
	buff+=mmdx.getChunk();

	mhdr.setOffsetMMID(buff.getSize());
	buff+=mmid.getChunk();

	mhdr.setOffsetMWMO(buff.getSize());
	buff+=mwmo.getChunk();

	mhdr.setOffsetMWID(buff.getSize());
	buff+=mwid.getChunk();

	mhdr.setOffsetMDDF(buff.getSize());
	buff+=mddf.getChunk();

	mhdr.setOffsetMODF(buff.getSize());
	buff+=modf.getChunk();
}

Buffer ADT::getFileBuff(){
	Render();
	this->buff.setPosition(0);
	return this->buff;
}

ADT::~ADT(){

}

}
}