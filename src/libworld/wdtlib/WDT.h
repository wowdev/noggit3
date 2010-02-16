/*
 * WDT.h
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#ifndef WDT_H_
#define WDT_H_
#include "../WoWFile.h"
#include "../MVER.h"
#include "MPHD.h"
#include "MAIN.h"
#include "../adtlib/MWMO.h"
#include "../adtlib/MODF.h"

namespace libworld
{
namespace wdtlib
{

class WDT:public WoWFile{
public:
	WDT();
	WDT(Buffer buff);
	~WDT();
	void Render();
	Buffer getFileBuff();

	bool hasTerrain();
	bool hasBigAlpha();

	void unsetTerrain();
	void setTerrain();

	bool hasADT(int i);
	bool hasADT(int x,int y);

	void setADT(int i);
	void setADT(int x,int y);

	void unsetADT(int i);
	void unsetADT(int x,int y);

	char *getWMOName();
	int getLengthWMOName();

	libworld::adtlib::MODF_Entry getWMOInfo();

private:
	MVER mver;
	MPHD mphd;
	MAIN main;
	libworld::adtlib::MWMO mwmo;
	libworld::adtlib::MODF modf;
};

}
}

#endif /* WDT_H_ */
