#ifndef MAPHEADERS
#define MAPHEADERS

#include <stdint.h>

enum eMCNKFlags
{
	FLAG_SHADOW = 0x1,
	FLAG_IMPASS	= 0x2,
	FLAG_LQ_RIVER	= 0x4,
	FLAG_LQ_OCEAN	= 0x8,
	FLAG_LQ_MAGMA	= 0x10,
	FLAG_LQ_SLIME	= 0x20,
	FLAG_MCCV = 0x40,
	FLAG_TBC = 0x8000
};

static const float TILESIZE = 533.33333f;
static const float CHUNKSIZE = ((TILESIZE) / 16.0f);
static const float UNITSIZE = (CHUNKSIZE / 8.0f);
static const float MINICHUNKSIZE = (CHUNKSIZE / 4.0f);
static const float ZEROPOINT = (32.0f * (TILESIZE));

struct MHDR
{
 /*000h*/	uint32_t flags;				// &1: MFBO, &2: unknown. in some Northrend ones.
 /*004h*/	uint32_t mcin;	//Positions of MCNK's
 /*008h*/	uint32_t mtex;	//List of all the textures used
 /*00Ch*/	uint32_t mmdx;	//List of all the md2's used
 /*010h*/	uint32_t mmid;	//Offsets into MMDX list for what each ID is
 /*014h*/	uint32_t mwmo;	//list of all the WMO's used
 /*018h*/	uint32_t mwid;	//Offsets into MWMO list for what each ID is
 /*01Ch*/	uint32_t mddf;	//Doodad Information
 /*020h*/	uint32_t modf;	//WMO Positioning Information
 /*024h*/	uint32_t mfbo;	// tbc, wotlk; only when flags&1
 /*028h*/	uint32_t mh2o;	// wotlk
 /*02Ch*/	uint32_t mtfx;	// wotlk
 /*030h*/	uint32_t pad4;		
 /*034h*/	uint32_t pad5;		
 /*038h*/	uint32_t pad6;		
 /*03Ch*/	uint32_t pad7;	
 /*040h*/
};

struct ENTRY_MCIN
{
	uint32_t	offset;
	uint32_t	size;
	uint32_t	flags;
	uint32_t	asyncID;
};

struct MCIN
{
	ENTRY_MCIN mEntries[256];
};

struct ENTRY_MDDF
{
	uint32_t	nameID;
	uint32_t	uniqueID;	
	float	pos[3];
	float	rot[3];
	//uint16_t	flags;
	uint16_t	scale;
	uint16_t	flags;
};

struct ENTRY_MODF
{
	uint32_t	nameID;
	uint32_t	uniqueID;	
	float	pos[3];
	float	rot[3];
	float	extents[2][3];
	//uint16_t	flags;
	uint16_t	flags;
	uint16_t	doodadSet;
	uint16_t	nameSet;
	uint16_t	unknown;
};

struct MapChunkHeader {
	uint32_t flags;
	uint32_t ix;
	uint32_t iy;
	uint32_t nLayers;
	uint32_t nDoodadRefs;
	uint32_t ofsHeight;
	uint32_t ofsNormal;
	uint32_t ofsLayer;
	uint32_t ofsRefs;
	uint32_t ofsAlpha;
	uint32_t sizeAlpha;
	uint32_t ofsShadow;
	uint32_t sizeShadow;
	uint32_t areaid;
	uint32_t nMapObjRefs;
	uint32_t holes;
	uint16_t s1;
	uint16_t s2;
	uint32_t d1;
	uint32_t d2;
	uint32_t d3;
	uint32_t predTex;
	uint32_t nEffectDoodad;
	uint32_t ofsSndEmitters;
	uint32_t nSndEmitters;
	uint32_t ofsLiquid;
	uint32_t sizeLiquid;
	float	zpos;
	float	xpos;
	float	ypos;
	uint32_t ofsMCCV;
	uint32_t unused1;
	uint32_t unused2;
};

struct ENTRY_MCLY
{
	uint32_t	textureID;
	uint32_t	flags;
	uint32_t	ofsAlpha;
	uint32_t	effectID;
};

#include <string.h> // memcpy()
// are these used?

struct MH2O_Header{
	uint32_t ofsInformation;
	uint32_t nLayers;
	uint32_t ofsRenderMask;
};

struct MH2O_Information{
	uint16_t LiquidType;
	uint16_t Flags;
	float minHeight;//I just took these random ._.
	float maxHeight;
	uint8_t xOffset;
	uint8_t yOffset;
	uint8_t width;
	uint8_t height;
	uint32_t ofsInfoMask;
	uint32_t ofsHeightMap;
};

struct MH2O_HeightMask{
	float **mHeightValues;
	uint8_t **mTransparency;
	int mWidth;
	int mHeight;
	MH2O_HeightMask(int Width,int Height,char*file,int Position){
		mWidth=Width;
		mHeight=Height;
		mHeightValues=new float*[mHeight];
		for(int i=0;i<mHeight;++i)
			mHeightValues[i]=new float[mWidth];
		mTransparency=new uint8_t*[mHeight];
		for(int i=0;i<mHeight;++i)
			mTransparency[i]=new uint8_t[mWidth];
		for(int i=0;i<mHeight;++i)
			memcpy(mHeightValues[i],file+Position+i*mWidth*sizeof(float),mWidth*sizeof(float));
		for(int i=0;i<mHeight;++i)
			memcpy(mTransparency[i],file+Position+mWidth*mHeight*sizeof(float)+i*mWidth*sizeof(uint8_t),mWidth*sizeof(uint8_t));
	}
	MH2O_HeightMask(int Width,int Height,float*HeightValues,uint8_t *Transparency){
		mWidth=Width;
		mHeight=Height;
		mHeightValues=new float*[mHeight];
		for(int i=0;i<mHeight;++i)
			mHeightValues[i]=new float[mWidth];
		mTransparency=new uint8_t*[mHeight];
		for(int i=0;i<mHeight;++i)
			mTransparency[i]=new uint8_t[mWidth];
		for(int i=0;i<mHeight;++i)
			memcpy(mHeightValues[i],HeightValues+i*mWidth*sizeof(float),mWidth*sizeof(float));
		for(int i=0;i<mHeight;++i)
			memcpy(mTransparency[i],Transparency+i*mWidth*sizeof(uint8_t),mWidth*sizeof(uint8_t));
	}
	MH2O_HeightMask(int Width=0,int Height=0){
		mWidth=Width;
		mHeight=Height;
		mHeightValues=new float*[mHeight];
		for(int i=0;i<mHeight;++i)
			mHeightValues[i]=new float[mWidth];
		mTransparency=new uint8_t*[mHeight];
		for(int i=0;i<mHeight;++i)
			mTransparency[i]=new uint8_t[mWidth];
	}
};

struct MH2O_Render{
	bool mRender[64];
	MH2O_Render(){
		
	}
	MH2O_Render(uint64_t Mask){
		for(int i=0;i<64;++i){
			uint8_t t= Mask<<i;
			mRender[i]=((t&0x1)==1);
		}
	}
};

#endif
