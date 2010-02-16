/*
 * MAIN.h
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "../Chunk.h"

namespace libworld
{
namespace wdtlib
{

struct Main_Object{
	int flags;
	int AsyncObject;
};

class MAIN:public Chunk{
public:
	MAIN();
	MAIN(Buffer buff);
	~MAIN();
	void Render();
	Buffer getChunk();

	bool hasADT(int i);
	bool hasADT(int x,int y);

	void setADT(int i);
	void setADT(int x,int y);

	void unsetADT(int i);
	void unsetADT(int x,int y);

private:
	Main_Object table[64*64];
};

}
}

#endif /* MAIN_H_ */
