/*
 * MHDR.cpp
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#include "MHDR.h"

namespace libworld
{
namespace adtlib
{

MHDR::MHDR(){
	_Tn("MHDR::MHDR()");
	int Size=sizeof(MHDR_Entry)+data;
	char *f=new char[sizeof(MHDR_Entry)+data];
	Buffer b=Buffer(f,Size);
	this->buff=b;
	this->buff.setPosition(magic);
	char m[]={'R','D','H','M'};
	Magic=MagicToInt(m);
}

MHDR::MHDR(Buffer buff){
	_Tn("MHDR::MHDR(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(magic);
	this->buff >> Magic >> ChunkSize;
	this->buff.setPosition(data);
	this->buff >>info;
}

void MHDR::Render(){
	ChunkSize=sizeof(MHDR_Entry);
	this->buff=Buffer(data+ChunkSize);
	this->buff.setPosition(magic);
	this->buff<<Magic;
	this->buff<<ChunkSize;
	this->buff.setPosition(data);
	this->buff<<info;
}

Buffer MHDR::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MHDR::getOffsetMCIN(){
	return info.ofsMCIN+relative;
}

void MHDR::setOffsetMCIN(int val){
	info.ofsMCIN=val-relative;
}

int MHDR::getOffsetMDDF(){
	return info.ofsMDDF+relative;
}

void MHDR::setOffsetMDDF(int val){
	info.ofsMDDF=val-relative;
}

bool MHDR::MFBOExists(){
	return (info.ofsMFBO!=0&&info.flags&1);
}

int MHDR::getOffsetMFBO(){
	return info.ofsMFBO+relative;
}

void MHDR::setOffsetMFBO(int val){
	info.ofsMFBO=val-relative;
	info.flags&=1;
}

bool MHDR::MH2OExists(){
	return (info.ofsMH2O!=0);
}

int MHDR::getOffsetMH2O(){
	return info.ofsMH2O+relative;
}

void MHDR::setOffsetMH2O(int val){
	info.ofsMH2O=val-relative;
}

int MHDR::getOffsetMMDX(){
	return info.ofsMMDX+relative;
}

void MHDR::setOffsetMMDX(int val){
	info.ofsMMDX=val-relative;
}

int MHDR::getOffsetMMID(){
	return info.ofsMMID+relative;
}

void MHDR::setOffsetMMID(int val){
	info.ofsMMID=val-relative;
}

int MHDR::getOffsetMODF(){
	return info.ofsMODF+relative;
}

void MHDR::setOffsetMODF(int val){
	info.ofsMODF=val-relative;
}

int MHDR::getOffsetMTEX(){
	return info.ofsMTEX+relative;
}

void MHDR::setOffsetMTEX(int val){
	info.ofsMTEX=val-relative;
}

int MHDR::getOffsetMWID(){
	return info.ofsMWID+relative;
}
void MHDR::setOffsetMWID(int val){
	info.ofsMWID=val-relative;
}

int MHDR::getOffsetMWMO(){
	return info.ofsMWMO+relative;
}

void MHDR::setOffsetMWMO(int val){
	info.ofsMWMO=val-relative;
}

bool MHDR::MTFXExists(){
	//most likely &&flags&2
	return (info.ofsMTFX!=0);
}

int MHDR::getOffsetMTFX(){
	return info.ofsMTFX+relative;
}

void MHDR::setOffsetMTFX(int val){
	info.ofsMTFX=val-relative;
	//info.flags&=2 ???
}



MHDR::~MHDR(){

}

}
}