/*
 * MWID.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef MWID_H_
#define MWID_H_
#include "../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MWID:public Chunk{

public:
	MWID();
	MWID(Buffer buff);
	~MWID();
	void Render();
	Buffer getChunk();

	int getOffset(int i);
	void setOffset(int val,int i);

	int getnOffsets();
	void setnOffsets(int n);
	void addOffset(int n);

private:
	int nOffsets;
	int *ObjectOffsets;

};

}
}

#endif /* MWID_H_ */