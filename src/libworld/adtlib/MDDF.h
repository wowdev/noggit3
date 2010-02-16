/*
 * MMDF.h
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#ifndef MDDF_H_
#define MDDF_H_
#include "mapheaders.h"
#include "../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MDDF:public Chunk{
public:
	MDDF();
	MDDF(Buffer buff);
	~MDDF();
	void Render();
	Buffer getChunk();
private:
	MDDF_Entry *entries;
	int nEntries;
};

}
}

#endif /* MDDF_H_ */
