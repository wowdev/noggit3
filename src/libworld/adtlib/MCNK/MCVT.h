/*
 * MCVT.h
 *
 *  Created on: 01.08.2009
 *      Author: Bastian
 */

#ifndef MCVT_H_
#define MCVT_H_
#include "../../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MCVT:public Chunk{

public:
	MCVT(Buffer buff);
	MCVT();
	~MCVT();
	void Render();
	Buffer getChunk();

	float getValNoLOD(int x, int y);
	float getValLOD(int x, int y);
	float getVal(int index);

	void setValNoLOD(float val,int x, int y);
	void setValLOD(float val,int x, int y);
	void setVal(float val,int index);


private:
	float heightvalues[145];

};

}
}

#endif /* MCVT_H_ */
