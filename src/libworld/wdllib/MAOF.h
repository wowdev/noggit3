/*
 * MAOF.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef MAOF_H_
#define MAOF_H_
#include "../Chunk.h"

namespace libworld
{
namespace wdllib
{

class MAOF:public Chunk{

public:
	MAOF();
	MAOF(Buffer buff);
	~MAOF();
	void Render();
	Buffer getChunk();

	int getOffset(int index);
	int getOffset(int x,int y);

	void setOffset(int val,int index);
	void setOffset(int val,int x,int y);

private:
	int aOffset[4096];

};

}
}

#endif /* MAOF_H_ */
