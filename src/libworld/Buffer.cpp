/*
 * Buffer.cpp
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */
/***
 * ToDo:Make unsigned cause of << signing sometimes negative
 */

#include "Buffer.h"

namespace libworld
{

Buffer::Buffer(){
	//_Tn("Buffer::Buffer()");
	this->Size=0;
	this->buff=SMemNew(Size);
	this->Position=0;
}

Buffer::Buffer(char *file,int FileSize){
	//_T("Buffer::Buffer(char *file,","FileSize: %d)",FileSize);
	this->Size=FileSize;
	this->buff=SMemNew(FileSize);
	memcpy(this->buff,file,FileSize);
	this->Position=0;
}

Buffer::Buffer(char *file,int FileSize,int start){
	//_T("Buffer::Buffer(char *file",",Size: %d,Start: %d)",FileSize,start);
	this->Size=FileSize-start;
	this->buff=SMemNew(FileSize-start);
	memcpy(this->buff,file+start,FileSize-start);
	this->Position=0;

}

Buffer::Buffer(Buffer buff,int start){
	//_T("Buffer::Buffer(Buffer buff,","Start:%d)",start);
	//for chunks!
	int cs=buff.get<int>(start+4);
	this->Size=cs+8;
	this->buff=SMemNew(Size);
	memcpy(this->buff,buff.getData()+start,Size);
	this->Position=0;
}

Buffer::Buffer(int Size){
	//_T("Buffer::Buffer(Size: ","%d)",Size);
	this->Size=Size;
	this->buff=SMemNew(Size);
	this->Position=0;
}

void Buffer::deleteBuffer(){
	Size=0;
	Position=0;
	SMemDelete(buff);
}

int Buffer::getSize(){
	_TDn("int Buffer::getSize()");
	return this->Size;
}

char *Buffer::getData(){
	_TDn("char *Buffer::getData()");
	setPosition(0);
	return this->buff;
}

bool Buffer::setPosition(int position){
	_TD("bool Buffer::setPosition(","Pos: %d)",position);
	this->Position=position;
	return true;
}

bool Buffer::hasRemaining(){
	_TDn("bool Buffer::hasRemaining()");
	return(this->Position<this->Size ? true :  false);
}

/*
 * Sets the Position to the Beginning of the Buff!
 */
Buffer& Buffer::operator +(const Buffer &v){
	_TDn("Buffer& Buffer::operator +(const Buffer &v)");
	char *temp=buff;
	int newSize=this->Size+v.Size;
	buff=SMemNew(newSize);
	memcpy(buff,temp,this->Size);
	SMemDelete(temp);
	memcpy(buff+this->Size,v.buff,v.Size);
	this->Size+=v.Size;
	this->Position=0;
	return *this;
}

Buffer& Buffer::operator =(const Buffer &v){
	_TDn("Buffer& Buffer::operator =(const Buffer &v)");
	this->buff=v.buff;
	this->Position=v.Position;
	this->Size=v.Size;
	return *this;
}

Buffer& Buffer::operator +=(const Buffer &v){
	_TDn("Buffer& Buffer::operator +=(const Buffer &v)");
	Buffer &t=*this+v;
	return t;
}

Buffer::~Buffer(){
	_TDn("~Buffer()");
}

}
