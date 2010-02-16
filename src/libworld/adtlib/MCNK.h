/*
 * MCNK.h
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */

#ifndef MCNK_H_
#define MCNK_H_

#include "../Chunk.h"
#include "mapheaders.h"

#include "MCNK/MCVT.h"
#include "MCNK/MCLY.h"

namespace libworld
{
namespace adtlib
{

class MCNK:public Chunk{

public:
	MCNK();
	MCNK(Buffer buff);
	~MCNK();
	void Render();
	Buffer getChunk();


	void removeMCSE();


private:
	MCNK_Header info;
	/*
	 * optional Chunks:
	 * MCCV
	 * MCSH
	 * MCLQ
	 * MCSE
	 */
	MCVT mcvt;
	MCLY mcly;
};

}
}

#endif /* MCNK_H_ */
