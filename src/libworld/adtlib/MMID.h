/*
 * MWID.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef MMID_H_
#define MMID_H_
#include "../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MMID:public Chunk{

public:
	MMID();
	MMID(Buffer buff);
	~MMID();
	void Render();
	Buffer getChunk();

	int getOffset(int i);
	void setOffset(int val,int i);

	int getnOffsets();
	void setnOffsets(int n);
	void addOffset(int n);

private:
	int nOffsets;
	int *DoodadOffsets;

};

}
}

#endif /* MMID_H_ */
