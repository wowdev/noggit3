/*
 * MARE.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef MARE_H_
#define MARE_H_
#include "../Chunk.h"

namespace libworld
{
namespace wdllib
{

class MARE:public Chunk{
public:
	MARE();
	MARE(Buffer buff);
	~MARE();
	void Render();

	short getInnerHeight(int index);
	short getInnerHeight(int x,int y);
	void setInnerHeight(short val,int index);
	void setInnerHeight(short val,int x,int y);

	short getOuterHeight(int index);
	short getOuterHeight(int x,int y);
	void setOuterHeight(short val,int index);
	void setOuterHeight(short val,int x,int y);

private:
	short OuterHeight[17*17];
	short InnerHeight[16*16];


};

}
}

#endif /* MARE_H_ */
