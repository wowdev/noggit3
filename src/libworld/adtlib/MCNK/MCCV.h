/*
 * MCVT.h
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */

#ifndef MCCV_H_
#define MCCV_H_
#include "../../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MCCV:public Chunk{

public:
	MCCV(Buffer buff);
	MCCV();
	~MCCV();
	void Render();
	Buffer getChunk();

	int getValNoLOD(int x, int y);
	int getValLOD(int x, int y);
	int getVal(int index);

	void setValNoLOD(int val,int x, int y);
	void setValLOD(int val,int x, int y);
	void setVal(int val,int index);



private:
	int colorvalues[145];

};

}
}

#endif /* MCCV_H_ */
