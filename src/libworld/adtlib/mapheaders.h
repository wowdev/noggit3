/*
 * mapheaders.h
 *
 *  Created on: 02.08.2009
 *      Author: Bastian
 */

#ifndef MAPHEADERS_H_
#define MAPHEADERS_H_

namespace libworld
{
namespace adtlib
{

struct MHDR_Entry{

	/*000h*/  int flags;
	/*004h*/  int ofsMCIN;
	/*008h*/  int ofsMTEX;
	/*00Ch*/  int ofsMMDX;
	/*010h*/  int ofsMMID;
	/*014h*/  int ofsMWMO;
	/*018h*/  int ofsMWID;
	/*01Ch*/  int ofsMDDF;
	/*020h*/  int ofsMODF;
	/*024h*/  int ofsMFBO; // tbc, wotlk
	/*028h*/  int ofsMH2O;		// new in WotLK
	/*02Ch*/  int ofsMTFX;//wotlk
	/*030h*/  int pad4;
	/*034h*/  int pad5;
	/*038h*/  int pad6;
	/*03Ch*/  int pad7;
	/*040h*/
};

enum MHDR_Flags{
	MFBO_Exists	=	0x1,
	MTFX_Exists	=	0x2 //I guess, not sure about
};

struct MCIN_Entry{
	int ofsMCNK;
	int sizeMCNK;
	int flags;
	int asyncID;
};

struct MDDF_Entry{
	/*000h*/  int nameId;
	/*004h*/  int uniqueId;
	/*008h*/  float pos[3];
	/*00Ch*/
	/*010h*/
	/*014h*/  float rot[3];
	/*018h*/
	/*01Ch*/
	/*020h*/  short scale;
	/*022h*/  short flags;
	/*024h*/
};

struct MODF_Entry{
	/*000h*/  int nameId;
	/*004h*/  int uniqueId;
	/*008h*/  float pos[3];
	/*00Ch*/
	/*010h*/
	/*014h*/  float rot[3];
	/*018h*/
	/*01Ch*/
	/*020h*/  float extents[6];
	/*024h*/
	/*028h*/
	/*02Ch*/
	/*030h*/
	/*034h*/
	/*038h*/  short flags;
	/*03Ch*/  short doodadSet;
	/*03Eh*/  short nameSet;
	/*040h*/  short unk;
};

struct MCNK_Header{
	/*000h*/  int flags;
	/*004h*/  int IndexX;
	/*008h*/  int IndexY;
	/*00Ch*/  int nLayers;
	/*010h*/  int nDoodadRefs;
	/*014h*/  int ofsMCVT;
	/*018h*/  int ofsMCNR;
	/*01Ch*/  int ofsMCLY;
	/*020h*/  int ofsMCRF;
	/*024h*/  int ofsMCAL;
	/*028h*/  int sizeAlpha;
	/*02Ch*/  int ofsMCSH;
	/*030h*/  int sizeShadow;
	/*034h*/  int areaid;
	/*038h*/  int nMapObjRefs;
	/*03Ch*/  int holes;
	/*040h*/  short unk1;
	/*042h*/  short unk2;
	/*044h*/  int unk3;
	/*048h*/  int unk4;
	/*04Ch*/  int unk5;
	/*050h*/  int predTex;
	/*054h*/  int noEffectDoodad;
	/*058h*/  int ofsMCSE;
	/*05Ch*/  int nSndEmitters;
	/*060h*/  int ofsMCLQ;
	/*064h*/  int sizeLiquid;
	/*068h*/  float  pos[3];
	/*06Ch*/
	/*070h*/
	/*074h*/  int ofsMCCV; //textureId;
	/*078h*/  int props;
	/*07Ch*/  int effectId;
	/*080h*/
};

enum MCNK_Flags{
	MCSH_Available	= 0x1,
	Impassable	=	0x2,
	River	=	0x4,
	Ocean	=	0x8,
	Magma	=	0x10,
	Slime	=	0x20,
	MCCV_Available	=	0x40,
	Unk_Used_TBC	=	0x8000
};

struct MCLY_Entry{
	int TextureID;
	int Flags;
	int ofsAlphaMap;
	int GroundEffect;
};

enum MCLY_Flags{
	UseAlphaMap	=	0x100,
	AlphaCompressed	=	0x200,
	SkyReflection	=	0x400
};

struct MCSE_Entry{
	/*000h*/  int soundPointID;
	/*004h*/  int soundNameID;
	/*008h*/  float  pos[3];
	/*00Ch*/
	/*010h*/
	/*014h*/  float minDistance;
	/*018h*/  float maxDistance;
	/*01Ch*/  float cutoffDistance;
	/*020h*/  short startTime;
	/*022h*/  short endTime;
	/*024h*/  short groupSilenceMin;
	/*026h*/  short groupSilenceMax;
	/*028h*/  short playInstancesMin;
	/*02Ah*/  short playInstancesMax;
	/*02Ch*/  short loopCountMin;
	/*02Eh*/  short loopCountMax;
	/*030h*/  short interSoundGapMin;
	/*032h*/  short interSoundGapMax;
	/*034h*/
};

}
}

#endif /* MAPHEADERS_H_ */
