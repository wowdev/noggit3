/*
 * MCLY.h
 *
 *  Created on: 09.08.2009
 *      Author: Bastian
 */

#ifndef MCLY_H_
#define MCLY_H_
#include "../../Chunk.h"
#include "../mapheaders.h"

namespace libworld
{
namespace adtlib
{

class MCLY:public Chunk{
public:
	MCLY(Buffer buff);
	MCLY();
	~MCLY();

	MCLY_Entry getEntry(int n);
	bool addEntry(MCLY_Entry add);

private:
	MCLY_Entry *Entries;
	int nEntries;
};

}
}

#endif /* MCLY_H_ */
