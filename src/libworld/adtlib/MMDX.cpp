/*
 * MMDX.cpp
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#include "MMDX.h"

namespace libworld
{
namespace adtlib
{

MMDX::MMDX(){
	buff=Buffer(data);
	char m[]={'X','D','M','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MMDX::MMDX(Buffer buff){
		_Tn("MMDX::MMDX(Buffer buff)");
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		nDoodads=0;
		this->buff.setPosition(data);
		int it=0;
		do{
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			it++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				it++;
			}while(c!='\0');
			nDoodads++;
		}while(it<ChunkSize);

		lenDoodad=new int[nDoodads];
		doodadname=new char*[nDoodads];
		ofsDoodad=new int[nDoodads];

		it=0;
		for(int i=0;i<nDoodads;i++){
			ofsDoodad[i]=it;
			lenDoodad[i]=0;
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			it++;
			lenDoodad[i]++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				it++;
				lenDoodad[i]++;
			}while(c!='\0');
			doodadname[i]=SMemNew(lenDoodad[i]);
		}
		it=0;
		for(int i=0;i<nDoodads;i++){
			int pos=0;
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			doodadname[i][pos]=c;
			it++;
			pos++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				doodadname[i][pos]=c;
				it++;
				pos++;

			}while(c!='\0');
		}


}

void MMDX::Render(){
	//calc needed size
	int size=0;
	for(int i=0;i<nDoodads;i++){
		size+=lenDoodad[i];
	//	size++;
	}
	this->buff=Buffer(data+size);
	this->buff<<Magic<<ChunkSize;
	for(int i=0;i<nDoodads;i++){
		for(int k=0;k<lenDoodad[i];k++){
			this->buff<<doodadname[i][k];
		}
	}
}

Buffer MMDX::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MMDX::getnDoodads(){
	_TDn("int MMDX::getnDoodads()");
	return nDoodads;
}

int MMDX::getofsDoodad(int n){
	_TDn("int MMDX::getofsDoodad(int n)");
	return ofsDoodad[n];
}

char* MMDX::getDoodadName(int n){
	_TD("char* MMDX::getDoodadName(","%d)",n);
	return doodadname[n];
}

int MMDX::getlenDoodadName(int n){
	_TD("int MMDX::getlenDoodadName(","%d)",n);
	return lenDoodad[n];
}

void MMDX::addDoodad(char *filename,int length){
	int *o=new int[nDoodads+1];
	char **n=new char*[nDoodads+1];
	int *l=new int[nDoodads+1];
	for(int i=0;i<nDoodads;i++){
		o[i]=ofsDoodad[i];
		n[i]=doodadname[i];
		l[i]=lenDoodad[i];
	}
	o[nDoodads]=ChunkSize;
	ChunkSize+=length;
	l[nDoodads]=length;
	n[nDoodads]=filename;
	ofsDoodad=o;
	doodadname=n;
	lenDoodad=l;
	nDoodads++;
}

MMDX::~MMDX(){

}

}
}