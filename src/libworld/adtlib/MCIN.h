/*
 * MCIN.h
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */

#ifndef MCIN_H_
#define MCIN_H_
#include "../Chunk.h"
#include "mapheaders.h"

namespace libworld
{
namespace adtlib
{

class MCIN:public Chunk{

public:
	MCIN();
	MCIN(Buffer buff);
	~MCIN();
	void Render();
	Buffer getChunk();

	MCIN_Entry getMCNKInfo(int x,int y);
	MCIN_Entry getMCNKInfo(int n);

private:
	MCIN_Entry Info[256];

};

}
}

#endif /* MCIN_H_ */
