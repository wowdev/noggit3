/*
 * MHDR.h
 *
 *  Created on: 31.07.2009
 *      Author: Bastian
 */

#ifndef MHDR_H_
#define MHDR_H_
#include "../Chunk.h"
#include "mapheaders.h"

namespace libworld
{
namespace adtlib
{

class MHDR:public Chunk{
public:
	const static int relative=0x14;

	 /*000h*/  const static int pad 			= data + 0x0;
	 /*004h*/  const static int offsMCIN 		= data + 0x4;
	 /*008h*/  const static int offsMTEX 		= data + 0x8;
	 /*00Ch*/  const static int offsMMDX		= data + 0xC;
	 /*010h*/  const static int offsMMID 		= data + 0x10;
	 /*014h*/  const static int offsMWMO 		= data + 0x14;
	 /*018h*/  const static int offsMWID		= data + 0x18;
	 /*01Ch*/  const static int offsMDDF	 	= data + 0x1C;
	 /*020h*/  const static int offsMODF 		= data + 0x20;
	 /*024h*/  const static int offsMFBO		= data + 0x24;
	 /*028h*/  const static int offsMH2O 		= data + 0x28;

	 /*02Ch*/  const static int offsMTFX = 0x34;
	 /*030h*/  const static int pad4 = 0x38;
	 /*034h*/  const static int pad5 = 0x3c;
	 /*038h*/  const static int pad6 = 0x40;
	 /*03Ch*/  const static int pad7 = 0x44;

	 MHDR();
	 MHDR(Buffer buff);
	 ~MHDR();
	 void Render();
	 Buffer getChunk();

	 /*
	  * Note:This will return the real offsets
	  * aka offset in MHDR+0x14!
	  */
	 int getOffsetMCIN();
	 void setOffsetMCIN(int val);

	 int getOffsetMTEX();
	 void setOffsetMTEX(int val);

	 int getOffsetMMDX();
	 void setOffsetMMDX(int val);

	 int getOffsetMMID();
	 int getOffsetMWMO();
	 int getOffsetMWID();
	 int getOffsetMDDF();
	 int getOffsetMODF();

	 void setOffsetMMID(int val);
	 void setOffsetMWMO(int val);
	 void setOffsetMWID(int val);
	 void setOffsetMDDF(int val);
	 void setOffsetMODF(int val);

	 bool MFBOExists();
	 int getOffsetMFBO();
	 void setOffsetMFBO(int val);

	 bool MH2OExists();
	 int getOffsetMH2O();
	 void setOffsetMH2O(int val);

	 bool MTFXExists();
	 int getOffsetMTFX();
	 void setOffsetMTFX(int val);


private:

	MHDR_Entry info;


};

}
}

#endif /* MHDR_H_ */
