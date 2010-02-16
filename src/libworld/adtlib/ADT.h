/*
 * ADT.h
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#ifndef ADT_H_
#define ADT_H_
#include "../WoWFile.h"
#include "../MVER.h"
#include "MHDR.h"
#include "MCIN.h"
#include "MTEX.h"
#include "MMDX.h"
#include "MMID.h"
#include "MWMO.h"
#include "MWID.h"
#include "MDDF.h"
#include "MODF.h"

#include "MCNK.h"

namespace libworld
{
namespace adtlib
{

class ADT: public WoWFile{
public:
	ADT();
	ADT(Buffer buff);
	~ADT();
	void Render();
	Buffer getFileBuff();
private:
	MVER mver;
	MHDR mhdr;
	MCIN mcin;
	MTEX mtex;
	MMDX mmdx;
	MMID mmid;
	MWMO mwmo;
	MWID mwid;
	MDDF mddf;
	MODF modf;

	MCNK mcnk[256];
};

}
}

#endif /* ADT_H_ */
