/*
 * MVER.h
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#ifndef MVER_H_
#define MVER_H_
#include "Chunk.h"

namespace libworld
{

class MVER:public Chunk{

public:
	const static int version = 0x8;

	MVER();
	MVER(Buffer buff);
	~MVER();
	void Render();
	Buffer getChunk();

	int getVersion();
	void setVersion(int vers);

private:
	int Version;
};

}

#endif /* MVER_H_ */
