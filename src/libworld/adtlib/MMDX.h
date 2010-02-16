/*
 * MMDX.h
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#ifndef MMDX_H_
#define MMDX_H_
#include "../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MMDX:public Chunk{

public:
	MMDX();
	MMDX(Buffer buff);
	~MMDX();
	void Render();
	Buffer getChunk();

	int getnDoodads();
	int getlenDoodadName(int n);
	char* getDoodadName(int n);
	int getofsDoodad(int n);

	void addDoodad(char *filename,int length);

private:
	char **doodadname;
	int nDoodads;
	int *lenDoodad;
	int *ofsDoodad;

};

}
}

#endif /* MMDX_H_ */
