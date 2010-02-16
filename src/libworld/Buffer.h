/*
 * Buffer.h
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#ifndef BUFFER_H_
#define BUFFER_H_
#include "SMem.h"
#include "trace.h"

namespace libworld
{

class Buffer{
public:
	Buffer();
	Buffer(char *file,int FileSize);
	Buffer(char *file,int FileSize,int start);
	Buffer(Buffer buff,int start);
	Buffer(int Size);
	~Buffer();

	void deleteBuffer();

	bool setPosition(int position);
	int getSize();
	char *getData();
	bool hasRemaining();

	template<class T>
	T get(){
		T ret=get<T>(this->Position);
		return ret;
	}
	template<class T>
	T get(int position){
		_TD("T get","(Position: %d)",position);
		T*ret=new T[1];
		ret=(T*)(buff+position);
		this->Position+=sizeof(T);
		return ret[0];
	}

	template<class T>
	void put(T in){
		put<T>(in,this->Position);
	}
	template<class T>
	void put(T in,int position){
		_TD("void put(T in,int position:","%d)",position);
		T *t=new T[1];
		t[0]=in;
		memcpy(buff+position,t,sizeof(T));
		this->Position+=sizeof(T);
	}

	template<class T>
	void insert(T in){
		insert<T>(in,this->Position);
	}
	template<class T>
	void insert(T in,int position){
		_TD("void insert(T in,position: ","%d)",position);
		char *temp=buff;
		buff=SMemNew(Size+sizeof(T));
		memcpy(buff,temp,position);
		T *t=new T[1];
		t[0]=in;
		memcpy(buff+position,t,sizeof(T));
		memcpy(buff+position+sizeof(T),temp+position,Size-position);
		Size++;
	}


	Buffer& operator+(const Buffer &v);
	Buffer& operator=(const Buffer &v);
	Buffer& operator+=(const Buffer &v);
	template<class T>
	Buffer& operator>>( T &value ) {
		_TDn("Buffer& operator>>( T &value )");
		value=get<T>();
		return *this;
	}
	template<class T>
	Buffer& operator<<( T &value ) {
		_TDn("Buffer& operator<<( T &value )");
		put<T>(value);
		return *this;
	}

	Buffer& operator<<(Buffer &value){
		memcpy(buff+Position,value.getData(),value.getSize());
		return *this;
	}


private:

	const static int FLOAT_SIZE=4;
	const static int INT_SIZE=4;
	const static int SHORT_SIZE=2;
	char *buff;
	int Size;
	int Position;
};

}

#endif /* BUFFER_H_ */