/*
 * MTEX.cpp
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#include "MTEX.h"

namespace libworld
{
namespace adtlib
{

MTEX::MTEX(){
	buff=Buffer(data);
	char m[]={'X','E','T','M'};
	Magic=MagicToInt(m);
	ChunkSize=0;
}

MTEX::MTEX(Buffer buff){
		_Tn("MTEX::MTEX(Buffer buff)");
		this->buff=buff;
		this->buff.setPosition(magic);
		this->buff >> Magic >> ChunkSize;
		nTex=0;
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
			nTex++;
		}while(it<ChunkSize);

		lenTex=new int[nTex];
		texname=new char*[nTex];

		it=0;
		for(int i=0;i<nTex;i++){
			lenTex[i]=0;
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			it++;
			lenTex[i]++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				it++;
				lenTex[i]++;
			}while(c!='\0');
			texname[i]=SMemNew(lenTex[i]);
		}
		it=0;
		for(int i=0;i<nTex;i++){
			int pos=0;
			char c;
			this->buff.setPosition(data+it);
			this->buff>>c;
			texname[i][pos]=c;
			it++;
			pos++;
			do{
				this->buff.setPosition(data+it);
				this->buff>>c;
				texname[i][pos]=c;
				it++;
				pos++;

			}while(c!='\0');
		}
}

void MTEX::Render(){
	//calc needed size
	int size=0;
	for(int i=0;i<nTex;i++){
		size+=lenTex[i];
	//	size++;
	}
	this->buff=Buffer(data+size);
	this->buff<<Magic<<ChunkSize;
	for(int i=0;i<nTex;i++){
		for(int k=0;k<lenTex[i];k++){
			this->buff<<texname[i][k];
		}
	}
}

Buffer MTEX::getChunk(){
	Render();
	this->buff.setPosition(magic);
	return this->buff;
}

int MTEX::getnTex(){
	_TDn("int MTEX::getnTex()");
	return nTex;
}

char* MTEX::getTexName(int n){
	_TD("char* MTEX::getTexName(","%d)",n);
	return texname[n];
}

int MTEX::getlenTexName(int n){
	_TD("int MTEX::getlenTexName(","%d)",n);
	return lenTex[n];
}

void MTEX::addTexture(char *filename,int length){
	char **n=new char*[nTex+1];
	int *l=new int[nTex+1];
	for(int i=0;i<nTex;i++){
		n[i]=texname[i];
		l[i]=lenTex[i];
	}
	l[nTex]=length;
	n[nTex]=filename;
	texname=n;
	lenTex=l;
	nTex++;
}

MTEX::~MTEX(){

}

}
}