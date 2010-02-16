/*
 * WoWFile.h
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */

#ifndef WOWFILE_H_
#define WOWFILE_H_
#include "Buffer.h"
#include "trace.h"
#include "MVER.h"

namespace libworld
{

class WoWFile{
public:
	WoWFile();
	WoWFile(Buffer buff);
	~WoWFile();

	void Render();

	Buffer getFileBuff();

protected:
	Buffer buff;
	MVER mver;

};

}

#endif /* WOWFILE_H_ */
