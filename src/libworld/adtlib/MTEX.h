/*
 * MTEX.h
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#ifndef MTEX_H_
#define MTEX_H_
#include "../Chunk.h"

namespace libworld
{
namespace adtlib
{

class MTEX:public Chunk{

public:
	MTEX();
	MTEX(Buffer buff);
	~MTEX();
	void Render();
	Buffer getChunk();

	int getnTex();
	int getlenTexName(int n);
	char* getTexName(int n);

	void addTexture(char *filename,int length);

private:
	char **texname;
	int nTex;
	int *lenTex;

};

}
}

#endif /* MTEX_H_ */
