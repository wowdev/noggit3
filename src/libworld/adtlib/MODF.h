/*
 * MODF.h
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#ifndef MODF_H_
#define MODF_H_
#include "../Chunk.h"
#include "mapheaders.h"


namespace libworld
{
namespace adtlib
{

class MODF:public Chunk{
public:
	MODF();
	MODF(Buffer buff);
	~MODF();
	void Render();
	Buffer getChunk();

	MODF_Entry getEntry(int i);
	int getnEntries();
	void addEntry(MODF_Entry val);

private:
	MODF_Entry *entries;
	int nEntries;
};


}
}

#endif /* MODF_H_ */
