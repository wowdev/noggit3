/*
 * MCNR.h
 *
 *  Created on: 26.08.2009
 *      Author: Bastian
 */

#ifndef MCNR_H_
#define MCNR_H_

#include "../../Chunk.h"

namespace libworld
{
namespace adtlib
{

struct Normal{
	char x;
	char y;
	char z;
};

class MCNR: public Chunk {
public:
	MCNR();
	MCNR(Buffer buff);
	~MCNR();

private:
	Normal normals[145];
};

}
}

#endif /* MCNR_H_ */
