/*
 * Chunk.h
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#ifndef CHUNK_H_
#define CHUNK_H_

#include "Buffer.h"
#include "SMem.h"
#include "trace.h"

namespace libworld
{

class Chunk{
public:
	const static int magic=0x0;
	const static int chunksize=0x4;
	const static int data=0x8;

	Chunk();
	Chunk(Buffer buff);
	~Chunk();

	bool MagicCorrect();
	void setMagic(int m);
	int getMagic();

	void setChunkSize(int Size);
	int getChunkSize();

	int getRealSize();

	Buffer getChunk();

protected:
	int Magic;
	int ChunkSize;
	Buffer buff;

	int MagicToInt(char magic[4]);

	void Render();
};

}

#endif /* CHUNK_H_ */
