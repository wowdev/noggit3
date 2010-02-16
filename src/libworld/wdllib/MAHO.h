/*
 * MAHO.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef MAHO_H_
#define MAHO_H_
#include "../Chunk.h"

namespace libworld
{
namespace wdllib
{

class MAHO:public Chunk{
public:
	MAHO();
	MAHO(Buffer buff);
	~MAHO();
	void Render();
	bool MagicCorrect();

	short getMask(int n);
	void setMask(short val,int n);

private:
	short mask[16];
};

}
}

#endif /* MAHO_H_ */
