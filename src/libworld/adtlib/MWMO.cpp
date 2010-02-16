/*
 * MWMO.cpp
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#include "MWMO.h"

namespace libworld
{
namespace adtlib
{

MWMO::MWMO(){
	buff=Buffer(data);
	char m[]={'O','M','W','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MWMO::MWMO(Buffer buff){
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		nObjects=0;
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
			nObjects++;
		}while(it<ChunkSize);

		lenObject=new int[nObjects];
		objectname=new char*[nObjects];
		ofsObject=new int[nObjects];

		it=0;
		for(int i=0;i<nObjects;i++){
			ofsObject[i]=it;
			lenObject[i]=0;
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			it++;
			lenObject[i]++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				it++;
				lenObject[i]++;
			}while(c!='\0');
			objectname[i]=SMemNew(lenObject[i]);
		}
		it=0;
		for(int i=0;i<nObjects;i++){
			int pos=0;
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			objectname[i][pos]=c;
			it++;
			pos++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				objectname[i][pos]=c;
				it++;
				pos++;

			}while(c!='\0');
		}


}

void MWMO::Render(){
	//calc needed size
	int size=0;
	for(int i=0;i<nObjects;i++){
		size+=lenObject[i];
	//	size++;
	}
	this->buff=Buffer(data+size);
	this->buff<<Magic<<ChunkSize;
	for(int i=0;i<nObjects;i++){
		for(int k=0;k<lenObject[i];k++){
			this->buff<<objectname[i][k];
		}
	}
}

Buffer MWMO::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MWMO::getnObjects(){
	_TDn("int MWMO::getnObjects()");
	return nObjects;
}

int MWMO::getofsObject(int n){
	_TDn("int MWMO::getofsObject(int n)");
	return ofsObject[n];
}

char* MWMO::getObjectName(int n){
	_TD("char* MWMO::getObjectName(","%d)",n);
	return objectname[n];
}

int MWMO::getlenObjectName(int n){
	_TD("int MWMO::getlenObjectName(","%d)",n);
	return lenObject[n];
}

void MWMO::addObject(char *filename,int length){
	int *o=new int[nObjects+1];
	char **n=new char*[nObjects+1];
	int *l=new int[nObjects+1];
	for(int i=0;i<nObjects;i++){
		o[i]=ofsObject[i];
		n[i]=objectname[i];
		l[i]=lenObject[i];
	}
	o[nObjects]=ChunkSize;
	ChunkSize+=length;
	l[nObjects]=length;
	n[nObjects]=filename;
	ofsObject=o;
	objectname=n;
	lenObject=l;
	nObjects++;
}

MWMO::~MWMO(){

}

}
}