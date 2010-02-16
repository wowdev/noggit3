/*
 * MWMO.h
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#ifndef MWMO_H_
#define MWMO_H_
#include "../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MWMO:public Chunk{

public:
	MWMO();
	MWMO(Buffer buff);
	~MWMO();
	void Render();
	Buffer getChunk();

	int getnObjects();
	int getlenObjectName(int n);
	char* getObjectName(int n);
	int getofsObject(int n);

	void addObject(char *filename,int length);

private:
	char **objectname;
	int nObjects;
	int *lenObject;
	int *ofsObject;

};

}
}

#endif /* MWMO_H_ */
