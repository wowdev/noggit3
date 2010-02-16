/*
 * WDL.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef WDL_H_
#define WDL_H_
#include "../WoWFile.h"
#include "MAOF.h"
#include "MAHO.h"
#include "MARE.h"

namespace libworld
{
namespace wdllib
{

class WDL:public WoWFile{
public:
	WDL();
	WDL(Buffer buff);
	~WDL();
	void Render();
	Buffer getFileBuff();
	unsigned int* createMinimap();

private:
	MVER mver;
	MAOF maof;
	bool maho_included;
	MAHO maho[4096];
	bool mare_exists[4096];
	MARE mare[4096];

};

}
}

#endif /* WDL_H_ */
