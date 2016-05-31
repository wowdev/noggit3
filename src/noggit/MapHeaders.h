// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cstdint>

enum WorldFlags {
    TERRAIN = 0x1,
    FOG = 0x2,
    DOODADS = 0x4,
    DRAWWMO = 0x8,
    WMODOODAS = 0x10,
    WATER = 0x20,
    LINES = 0x40,
    HOLELINES = 0x80,
    HEIGHTCONTOUR = 0x100,
    MARKIMPASSABLE = 0x200,
    AREAID = 0x400,
    NOCURSOR = 0x800
};

enum eMCNKFlags
{
  FLAG_SHADOW = 0x1,
  FLAG_IMPASS = 0x2,
  FLAG_LQ_RIVER = 0x4,
  FLAG_LQ_OCEAN = 0x8,
  FLAG_LQ_MAGMA = 0x10,
  FLAG_LQ_SLIME = 0x20,
  FLAG_MCCV = 0x40,
  FLAG_do_not_fix_alpha_map = 0x8000
};

enum eMCLYFlags
{
  FLAG_ANIMATE_45 = 0x1,
  FLAG_ANIMATE_90 = 0x2,
  FLAG_ANIMATE_180 = 0x4,
  FLAG_ANIM_FAST = 0x8,
  FLAG_ANIM_FASTER = 0x10,
  FLAG_ANIM_FASTEST = 0x20,
  FLAG_ANIMATE = 0x40,
  FLAG_GLOW = 0x80,
  FLAG_USE_ALPHA = 0x100, //should be set for every layer except the first
  FLAG_ALPHA_COMPRESSED = 0x200, //we do not compress, so ignore this
  FLAG_REFLECTION = 0x400
};

struct mcly_flags_type
{
  uint32_t animation_rotation : 3;
  uint32_t animation_speed : 3;
  uint32_t animate : 1;
  uint32_t glow : 1;
  uint32_t alpha_layer : 1;
  uint32_t compressed_alpha_layer : 1;
  uint32_t skybox_reflection : 1;
  uint32_t unused : 21;

  static mcly_flags_type& interpret (uint32_t& base)
  {
    return reinterpret_cast<mcly_flags_type&> (base);
  }
  static const mcly_flags_type& interpret (const uint32_t& base)
  {
    return reinterpret_cast<const mcly_flags_type&> (base);
  }
};

struct mcnk_flags_type
{
  uint32_t has_mcsh : 1;
  uint32_t impass : 1;
  uint32_t lq_river : 1;
  uint32_t lq_ocean : 1;
  uint32_t lq_magma : 1;
  uint32_t lq_slime : 1;
  uint32_t has_mccv : 1;
  uint32_t unknown_0x80 : 1;
  uint32_t unk0: 7;                      // not set in 6.2.0.20338
  uint32_t do_not_fix_alpha_map : 1; // "fix" alpha maps in MCAL (4 bit alpha maps are 63*63 instead of 64*64). Note that this also means that it *has* to be 4 bit alpha maps, otherwise UnpackAlphaShadowBits will assert.
  uint32_t high_res_holes : 1;       // Since ~5.3 WoW uses full 64-bit to store holes for each tile if this flag is set. The holes are a uint64_t at 0x14.
  uint32_t unk1 : 15;                     // not set in 6.2.0.20338

  static mcnk_flags_type& interpret(uint32_t& base)
  {
    return reinterpret_cast<mcnk_flags_type&> (base);
  }

  static const mcnk_flags_type& interpret(const uint32_t& base)
  {
    return reinterpret_cast<const mcnk_flags_type&> (base);
  }
};


static const float TILESIZE = 533.33333f;
static const float CHUNKSIZE = ((TILESIZE) / 16.0f);
static const float UNITSIZE = (CHUNKSIZE / 8.0f);
static const float MINICHUNKSIZE = (CHUNKSIZE / 4.0f);
static const float ZEROPOINT = (32.0f * (TILESIZE));

struct MHDR
{
  /*000h*/  uint32_t flags;        // &1: MFBO, &2: unknown. in some Northrend ones.
  /*004h*/  uint32_t mcin;  //Positions of MCNK's
  /*008h*/  uint32_t mtex;  //List of all the textures used
  /*00Ch*/  uint32_t mmdx;  //List of all the md2's used
  /*010h*/  uint32_t mmid;  //Offsets into MMDX list for what each ID is
  /*014h*/  uint32_t mwmo;  //list of all the WMO's used
  /*018h*/  uint32_t mwid;  //Offsets into MWMO list for what each ID is
  /*01Ch*/  uint32_t mddf;  //Doodad Information
  /*020h*/  uint32_t modf;  //WMO Positioning Information
  /*024h*/  uint32_t mfbo;  // tbc, wotlk; only when flags&1
  /*028h*/  uint32_t mh2o;  // wotlk
  /*02Ch*/  uint32_t mtfx;  // wotlk
  /*030h*/  uint32_t pad4;
  /*034h*/  uint32_t pad5;
  /*038h*/  uint32_t pad6;
  /*03Ch*/  uint32_t pad7;
  /*040h*/
};

struct ENTRY_MCIN
{
  uint32_t  offset;
  uint32_t  size;
  uint32_t  flags;
  uint32_t  asyncID;
};

struct MCIN
{
  ENTRY_MCIN mEntries[256];
};

struct ENTRY_MDDF
{
  uint32_t  nameID;
  uint32_t  uniqueID;
  float  pos[3];
  float  rot[3];
  //uint16_t  flags;
  uint16_t  scale;
  uint16_t  flags;
};

struct ENTRY_MODF
{
  uint32_t  nameID;
  uint32_t  uniqueID;
  float  pos[3];
  float  rot[3];
  float  extents[2][3];
  //uint16_t  flags;
  uint16_t  flags;
  uint16_t  doodadSet;
  uint16_t  nameSet;
  uint16_t  unknown;
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
  uint8_t low_quality_texture_map[0x10];
  uint32_t predTex;
  uint32_t nEffectDoodad;
  uint32_t ofsSndEmitters;
  uint32_t nSndEmitters;
  uint32_t ofsLiquid;
  uint32_t sizeLiquid;
  float  zpos;
  float  xpos;
  float  ypos;
  uint32_t ofsMCCV;
  uint32_t unused1;
  uint32_t unused2;
};

struct ENTRY_MCLY
{
  uint32_t  textureID;
  uint32_t  flags;
  uint32_t  ofsAlpha;
  uint32_t  effectID;
};

#include <string.h> // memcpy()

struct MH2O_Header
{
  uint32_t ofsInformation = 0;
  uint32_t nLayers = 0;
  uint32_t ofsRenderMask = 0;
};

struct MH2O_Information
{
  uint16_t LiquidType = 5;
  uint16_t Flags = 0;
  float minHeight = 0;//I just took these random ._.
  float maxHeight = 0;
  uint8_t xOffset = 0;
  uint8_t yOffset = 0;
  uint8_t width = 8;
  uint8_t height = 8;
  uint32_t ofsInfoMask = 0;
  uint32_t ofsHeightMap = 0;
};

struct MH2O_HeightMask
{
  float mHeightValues[9][9];
  unsigned char mTransparency[9][9];

  MH2O_HeightMask()
  {
    memset(mHeightValues, 0, sizeof(mHeightValues));
    memset(mTransparency, 0, sizeof(mTransparency));
  }
};

struct MH2O_Render
{
  unsigned char mask[8]; //render mask
  unsigned char fatigue[8]; //fatigue mask?

  MH2O_Render()
  {
    memset(mask, 255, sizeof (mask));
    memset(fatigue, 0, sizeof (fatigue));
  }
};
