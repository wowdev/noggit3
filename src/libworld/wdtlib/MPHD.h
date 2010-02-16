/*
 * MPHD.h
 *
 *  Created on: 23.08.2009
 *      Author: Bastian
 */

#ifndef MPHD_H_
#define MPHD_H_
#include "../Chunk.h"

namespace libworld
{
namespace wdtlib
{

enum MPHD_Flags{
	Map_No_Terrain=0x1,
	Map_Something=0x2,
	Map_TerrainShaders_BigAlpha=0x4,
	Map_Disable_Something=0x8
};

class MPHD:public Chunk{
public:
	MPHD();
	MPHD(Buffer buff);
	~MPHD();
	void Render();
	Buffer getChunk();

	bool hasTerrain();
	bool hasBigAlpha();

	void setTerrain();
	void unsetTerrain();

private:
	int flags;
	int something;
	int unused[6];
};

}
}

#endif /* MPHD_H_ */
