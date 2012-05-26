// MapChunk.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/MapChunk.h>

#include <algorithm>
#include <iostream>

#include <QMap>
#include <QSettings>

#include <math/random.h>
#include <math/vector_3d.h>

#include <opengl/texture.h>

#include <noggit/blp_texture.h>
#include <noggit/Brush.h>
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <noggit/mpq/file.h>

static const int HEIGHT_TOP = 1000;
static const int HEIGHT_MID = 600;
static const int HEIGHT_LOW = 300;
static const int HEIGHT_ZERO = 0;
static const int HEIGHT_SHALLOW = -100;
static const int HEIGHT_DEEP = -250;
static const double MAPCHUNK_DIAMETER  = 47.140452079103168293389624140323;
static const int CONTOUR_WIDTH = 128;
static const float texDetail = 8.0f;
static const float TEX_RANGE = 62.0f / 64.0f;


const int stripsize = 8*18 + 7*2;

StripType *mapstrip;
StripType *mapstrip2;

int indexMapBuf (int x, int y)
{
  return ((y+1)/2)*9 + (y/2)*8 + x;
}

int indexLoD(int x, int y)
{
  return (x+1)*9+x*8+y;
}

int indexNoLoD(int x, int y)
{
  return x*8+x*9+y;
}

// 8x8x2 version with triangle strips, size = 8*18 + 7*2
template <class V>
void stripify(V *in, V *out)
{
  for (int row=0; row<8; row++) {
    V *thisrow = &in[indexMapBuf(0,row*2)];
    V *nextrow = &in[indexMapBuf(0,(row+1)*2)];

    if (row>0) *out++ = thisrow[0];
    for (int col=0; col<9; col++) {
      *out++ = thisrow[col];
      *out++ = nextrow[col];
    }
    if (row<7) *out++ = nextrow[8];
  }
}

// high res version, size = 16*18 + 7*2 + 8*2
const int stripsize2 = 16*18 + 7*2 + 8*2;
template <class V>
void stripify2(V *in, V *out)
{
  for (int row=0; row<8; row++) {
    V *thisrow = &in[indexMapBuf(0,row*2)];
    V *nextrow = &in[indexMapBuf(0,row*2+1)];
    V *overrow = &in[indexMapBuf(0,(row+1)*2)];

    if (row>0) *out++ = thisrow[0];// jump end
    for (int col=0; col<8; col++) {
      *out++ = thisrow[col];
      *out++ = nextrow[col];
    }
    *out++ = thisrow[8];
    *out++ = overrow[8];
    *out++ = overrow[8];// jump start
    *out++ = thisrow[0];// jump end
    *out++ = thisrow[0];
    for (int col=0; col<8; col++) {
      *out++ = overrow[col];
      *out++ = nextrow[col];
    }
    if (row<8) *out++ = overrow[8];
    if (row<7) *out++ = overrow[8];// jump start
  }
}

void init_map_strip()
{
  // default strip indices
  StripType *defstrip = new StripType[stripsize];
  for (int i=0; i<stripsize; ++i) defstrip[i] = i; // note: this is ugly and should be handled in stripify
  mapstrip = new StripType[stripsize];
  stripify<StripType>(defstrip, mapstrip);
  delete[] defstrip;

  defstrip = new StripType[stripsize2];
  for (int i=0; i<stripsize2; ++i) defstrip[i] = i; // note: this is ugly and should be handled in stripify
  mapstrip2 = new StripType[stripsize2];
  stripify2<StripType>(defstrip, mapstrip2);
  delete[] defstrip;
}


/*
 White  1.00  1.00  1.00
 Brown  0.75  0.50  0.00
 Green  0.00  1.00  0.00
 Yellow  1.00  1.00  0.00
 Lt Blue  0.00  1.00  1.00
 Blue  0.00  0.00  1.00
 Black  0.00  0.00  0.00
 */

void HeightColor(float height, ::math::vector_3d *Color)
{
  float Amount;

  if(height>HEIGHT_TOP)
  {
    Color->x (1.0f);
    Color->y (1.0f);
    Color->z (1.0f);
  }
  else if(height>HEIGHT_MID)
  {
    Amount=(height-HEIGHT_MID)/(HEIGHT_TOP-HEIGHT_MID);
    Color->x (0.75f + Amount * 0.25f);
    Color->y (0.5f + 0.5f * Amount);
    Color->z (Amount);
  }
  else if(height>HEIGHT_LOW)
  {
    Amount=(height-HEIGHT_LOW)/(HEIGHT_MID-HEIGHT_LOW);
    Color->x (Amount * 0.75f);
    Color->y (1.00f - 0.5f * Amount);
    Color->z (0.0f);
  }
  else if(height>HEIGHT_ZERO)
  {
    Amount=(height-HEIGHT_ZERO)/(HEIGHT_LOW-HEIGHT_ZERO);

    Color->x (1.0f - Amount);
    Color->y (1.0f);
    Color->z (0.0f);
  }
  else if(height>HEIGHT_SHALLOW)
  {
    Amount=(height-HEIGHT_SHALLOW)/(HEIGHT_ZERO-HEIGHT_SHALLOW);
    Color->x (0.0f);
    Color->y (Amount);
    Color->z (1.0f);
  }
  else if(height>HEIGHT_DEEP)
  {
    Amount=(height-HEIGHT_DEEP)/(HEIGHT_SHALLOW-HEIGHT_DEEP);
    Color->x (0.0f);
    Color->y (0.0f);
    Color->z (Amount);
  }
  else
  {
    Color->x (0.0f);
    Color->y (0.0f);
    Color->z (0.0f);
  }
}



void MapChunk::GenerateContourMap()
{
  unsigned char  CTexture[CONTOUR_WIDTH*4];

  _contour_coord_gen[0]=0.0f;
  _contour_coord_gen[1]=0.25f;
  _contour_coord_gen[2]=0.0f;
  _contour_coord_gen[3]=0.0f;


  for(int i=0;i<(CONTOUR_WIDTH*4);++i)
    CTexture[i]=0;
  CTexture[3+CONTOUR_WIDTH/2]=0xff;
  CTexture[7+CONTOUR_WIDTH/2]=0xff;
  CTexture[11+CONTOUR_WIDTH/2]=0xff;

  glGenTextures(1, &_contour_texture);
  glBindTexture(GL_TEXTURE_2D, _contour_texture);


  gluBuild2DMipmaps(GL_TEXTURE_2D,4,CONTOUR_WIDTH,1,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);
  /*glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,CONTOUR_WIDTH,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

   for(int i=0;i<(CONTOUR_WIDTH*4)/2;++i)
   CTexture[i]=0;
   CTexture[3]=0xff;

   glTexImage1D(GL_TEXTURE_1D,1,GL_RGBA,CONTOUR_WIDTH/2,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

   for(int i=0;i<(CONTOUR_WIDTH*4)/4;++i)
   CTexture[i]=0;
   CTexture[3]=0x80;

   glTexImage1D(GL_TEXTURE_1D,2,GL_RGBA,CONTOUR_WIDTH/4,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

   for(int i=0;i<(CONTOUR_WIDTH*4)/8;++i)
   CTexture[i]=0;
   CTexture[3]=0x40;

   glTexImage1D(GL_TEXTURE_1D,3,GL_RGBA,CONTOUR_WIDTH/8,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

   for(int i=0;i<(CONTOUR_WIDTH*4)/16;++i)
   CTexture[i]=0;
   CTexture[3]=0x20;

   glTexImage1D(GL_TEXTURE_1D,4,GL_RGBA,CONTOUR_WIDTH/16,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

   for(int i=0;i<(CONTOUR_WIDTH*4)/32;++i)
   CTexture[i]=0;
   CTexture[3]=0x10;

   glTexImage1D(GL_TEXTURE_1D,5,GL_RGBA,CONTOUR_WIDTH/32,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

   for(int i=0;i<(CONTOUR_WIDTH*4)/64;++i)
   CTexture[i]=0;
   CTexture[3]=0x08;

   glTexImage1D(GL_TEXTURE_1D,6,GL_RGBA,CONTOUR_WIDTH/64,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);*/


  glEnable(GL_TEXTURE_GEN_S);
  glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  glTexGenfv(GL_S,GL_OBJECT_PLANE,_contour_coord_gen);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

//! \note I am aware of this being global state not even being inside a class BUT I DONT CARE AT ALL. THIS WHOLE THING IS BULLSHIT AND NOT WORTH A BIT AND COSTING ME TIME OF MY LIFE FOR JUST BEING BAD AS FUCK. REMOVEING ENVIRONMENT TOOK ME HOURS WHILE MOST VARIABLES HAD BEEN ONLY USED IN A SINGLE FUCKING CLASS,  DID NOT HAVE MEANINGFUL NAMES OR ANYTHING.
QMap<int, ::math::vector_3d> areaIDColors;

MapChunk::MapChunk(World* world, MapTile* maintile, noggit::mpq::file* f,bool bigAlpha)
  : _world (world)
  , _contour_texture (0)
{
  CreateStrips();
  init_map_strip();

  mt=maintile;
  mBigAlpha=bigAlpha;

  uint32_t fourcc;
  uint32_t size;

  f->read(&fourcc, 4);
  f->read(&size, 4);

  assert( fourcc == 'MCNK' );

  size_t lastpos = f->getPos() + size;

  f->read(&header, 0x80);

  Flags = header.flags;
  areaID = header.areaid;

  if (!areaIDColors.contains (areaID))
  {
    areaIDColors[areaID] = ::math::vector_3d ( ::math::random::floating_point (0.0f, 1.0f)
                                 , ::math::random::floating_point (0.0f, 1.0f)
                                 , ::math::random::floating_point (0.0f, 1.0f)
                                 );
  }

  zbase = header.zpos;
  xbase = header.xpos;
  ybase = header.ypos;

  px = header.ix;
  py = header.iy;

  holes = header.holes;

  /*
  if (hasholes) {
    gLog("Holes: %d\n", holes);
    int k=1;
    for (int j=0; j<4; j++) {
      for (int i=0; i<4; ++i) {
        gLog((holes & k)?"1":"0");
        k <<= 1;
      }
      gLog("\n");
    }
  }
  */

  // correct the x and z values ^_^
  zbase = zbase*-1.0f + ZEROPOINT;
  xbase = xbase*-1.0f + ZEROPOINT;

  vmin = ::math::vector_3d( 9999999.0f, 9999999.0f, 9999999.0f);
  vmax = ::math::vector_3d(-9999999.0f,-9999999.0f,-9999999.0f);
  glGenTextures(3, alphamaps);

  while (f->getPos() < lastpos) {
    f->read(&fourcc,4);
    f->read(&size, 4);

    size_t nextpos = f->getPos() + size;

    if ( fourcc == 'MCNR' ) {
      nextpos = f->getPos() + 0x1C0; // size fix
      // normal vectors
      char nor[3];
      ::math::vector_3d *ttn = mNormals;
      for (int j=0; j<17; ++j) {
        for (int i=0; i<((j%2)?8:9); ++i) {
          f->read(nor,3);
          // order X,Z,Y
          // *ttn++ = ::math::vector_3d((float)nor[0]/127.0f, (float)nor[2]/127.0f, (float)nor[1]/127.0f);
          *ttn++ = ::math::vector_3d(-nor[1]/127.0f, nor[2]/127.0f, -nor[0]/127.0f);
        }
      }
    }
    else if ( fourcc == 'MCVT' ) {
      ::math::vector_3d *ttv = mVertices;

      // vertices
      for (int j=0; j < 17; ++j) {
        for (int i=0; i < ((j % 2) ? 8 : 9); ++i) {
          float h,xpos,zpos;
          f->read(&h,4);
          xpos = i * UNITSIZE;
          zpos = j * 0.5f * UNITSIZE;
          if (j%2) {
                        xpos += UNITSIZE*0.5f;
          }
          ::math::vector_3d v (xbase+xpos, ybase+h, zbase+zpos);
          *ttv++ = v;
          vmin.y (std::min(vmin.y(), v.y()));
          vmax.y (std::max(vmax.y(), v.y()));
        }
      }

      vmin.x (xbase);
      vmin.z (zbase);
      vmax.x (xbase + 8 * UNITSIZE);
      vmax.z (zbase + 8 * UNITSIZE);
      r = (vmax - vmin).length() * 0.5f;

    }
    else if ( fourcc == 'MCLY' ) {
      // texture info
      nTextures = size / 16U;
      //gLog("=\n");
      for (size_t i=0; i<nTextures; ++i) {
        f->read(&tex[i],4);
        f->read(&texFlags[i], 4);
        f->read(&MCALoffset[i], 4);
        f->read(&effectID[i], 4);

        if (texFlags[i] & FLAG_ANIMATE) {
          animated[i] = texFlags[i];
        } else {
          animated[i] = 0;
        }
        _textures[i] = TextureManager::newTexture( mt->mTextureFilenames[tex[i]] );
      }
    }
    else if ( fourcc == 'MCSH' ) {
      // shadow map 64 x 64

      f->read( mShadowMap, 0x200 );
      f->seekRelative( -0x200 );

      unsigned char sbuf[64*64], *p, c[8];
      p = sbuf;
      for (int j=0; j<64; ++j) {
        f->read(c,8);
        for (int i=0; i<8; ++i) {
          for (int b = 0x01; b != 0x100; b <<= 1) {
            *p++ = (c[i] & b) ? 85 : 0;
          }
        }
      }
      glGenTextures(1, &shadow);
      glBindTexture(GL_TEXTURE_2D, shadow);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    }
    else if ( fourcc == 'MCAL' )
    {
      unsigned int MCALbase = f->getPos();
      for( unsigned int layer = 0; layer < header.nLayers; ++layer )
      {
        if( texFlags[layer] & 0x100 )
        {

          f->seek( MCALbase + MCALoffset[layer] );

          if( texFlags[layer] & 0x200 )
          {  // compressed

            // 21-10-2008 by Flow
            unsigned offI = 0; //offset IN buffer
            unsigned offO = 0; //offset OUT buffer
            char* buffIn = f->getPointer(); // pointer to data in adt file

            while( offO < 4096 )
            {
              // fill or copy mode
              bool fill = buffIn[offI] & 0x80;
              unsigned n = buffIn[offI] & 0x7F;
              offI++;
              for( unsigned k = 0; k < n; ++k )
              {
              if (offO == 4096) break;
              amap[layer-1][offO] = buffIn[offI];
              offO++;
              if( !fill )
                offI++;
              }
              if( fill ) offI++;
            }

            glBindTexture(GL_TEXTURE_2D, alphamaps[layer-1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[layer-1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          }
          else if(mBigAlpha){
            // not compressed
            unsigned char *p;
            char *abuf = f->getPointer();
            p = amap[layer-1];
            for (int j=0; j<64; ++j) {
              for (int i=0; i<64; ++i) {
                *p++ = *abuf++;
              }

            }

            memcpy(amap[layer-1]+63*64,amap[layer-1]+62*64,64);
            glBindTexture(GL_TEXTURE_2D, alphamaps[layer-1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[layer-1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            f->seekRelative(0x1000);
          }
          else
          {
            // not compressed
            unsigned char *p;
            char *abuf = f->getPointer();
            p = amap[layer-1];
            for (int j=0; j<63; ++j) {
              for (int i=0; i<32; ++i) {
                unsigned char c = *abuf++;
                *p++ = static_cast<unsigned char>((255*(static_cast<int>(c & 0x0f)))/0x0f);
                if(i != 31)
                {
                  *p++ = static_cast<unsigned char>((255*(static_cast<int>(c & 0xf0)))/0xf0);
                }
                else
                {
                  *p++ = static_cast<unsigned char>((255*(static_cast<int>(c & 0x0f)))/0x0f);
                }
              }

            }
            memcpy(amap[layer-1]+63*64,amap[layer-1]+62*64,64);
            glBindTexture(GL_TEXTURE_2D, alphamaps[layer-1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[layer-1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            f->seekRelative(0x800);
          }
        }
      }
    }
    else if ( fourcc == 'MCLQ' ) {
      // liquid / water level
      f->read(&fourcc,4);
      if( fourcc != 'MCSE' ||  fourcc != 'MCNK' || header.sizeLiquid == 8  || true ) { // Do not even try to read water..
        haswater = false;
      }
      else
      {
        //! \todo this is just commented out, till initFromTerrain is reimplemented for saving with MH2O!
        haswater = false;
      /*
        haswater = true;
        f->seekRelative(-4);
        float waterlevel[2];
        f->read(waterlevel,8);//2 values - Lowest water Level, Highest Water Level

        if (waterlevel[1] > vmax.y) vmax.y = waterlevel[1];
        //if (waterlevel < vmin.y) haswater = false;

        //f->seekRelative(4);

        Liquid * lq = new Liquid(8, 8, ::math::vector_3d(xbase, waterlevel[1], zbase));
        //lq->init(f);
        lq->initFromTerrain(f, header.flags);

        mt->mLiquids.insert(std::pair<int,Liquid*>( 0, lq) );


        // let's output some debug info! ( '-')b
        string lq = "";
        if (flags & 4) lq.append(" river");
        if (flags & 8) lq.append(" ocean");
        if (flags & 16) lq.append(" magma");
        if (flags & 32) lq.append(" slime?");
        LogDebug << "LQ" << lq << " (base:" << waterlevel << ")" << std::endl;
        */

      }
      // we're done here!
      break;
    }
    else if( fourcc == 'MCCV' )
    {
      //! \todo  implement
    }
    f->seek(nextpos);
  }

  // create vertex buffers
  glGenBuffers(1,&vertices);
  glGenBuffers(1,&normals);

  glBindBuffer(GL_ARRAY_BUFFER, vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, normals);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mNormals), mNormals, GL_STATIC_DRAW);

  initStrip();

  vcenter = (vmin + vmax) * 0.5f;

  nameID = _world->selection_names().add( this );



  ::math::vector_3d *ttv = mMinimap;

  // vertices
  for (int j=0; j<17; ++j) {
    for (int i=0; i < ((j % 2) ? 8 : 9); ++i) {
      float xpos,zpos;
        //f->read(&h,4);
      xpos = i * 0.125f;
      zpos = j * 0.5f * 0.125f;
      if (j % 2) {
                 xpos += 0.125f*0.5f;
      }
      ::math::vector_3d v = ::math::vector_3d(xpos+px, zpos+py, -1);
      *ttv++ = v;
    }
  }

  if( ( Flags & 1 ) == 0 )
  {
    /** We have no shadow map (MCSH), so we got no shadows at all!  **
     ** This results in everything being black.. Yay. Lets fake it! **/
    for( size_t i = 0; i < 512; ++i )
      mShadowMap[i] = 0;

    unsigned char sbuf[64*64];
    for( size_t j = 0; j < 4096; ++j )
      sbuf[j] = 0;

    glGenTextures( 1, &shadow );
    glBindTexture( GL_TEXTURE_2D, shadow );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  }

  for (size_t j (0); j < mapbufsize; ++j)
  {
    mFakeShadows[j].x (0.0f);
    mFakeShadows[j].y (0.0f);
    mFakeShadows[j].z (0.0f);
    mFakeShadows[j].w ( qBound ( 0.0f
                               , 1.0f - (-mNormals[j].x()
                                        + mNormals[j].y()
                                        - mNormals[j].z()
                                        )
                               , 1.0f
                               )
                      * 0.5f
                      );
  }

  glGenBuffers(1,&minimap);
  glGenBuffers(1,&minishadows);

  glBindBuffer(GL_ARRAY_BUFFER, minimap);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mMinimap), mMinimap, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, minishadows);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

void MapChunk::loadTextures()
{
  //! \todo Use this kind of preloading again?
  return;
/*  for(int i=0; i < nTextures; ++i)
    _textures[i] = TextureManager::get(mt->mTextureFilenames[tex[i]]);*/
}



void MapChunk::SetAnim (int anim, const float& anim_time) const
{
  if (anim) {
    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();

    // note: this is ad hoc and probably completely wrong
    const int spd = (anim & 0x08) | ((anim & 0x10) >> 2) | ((anim & 0x20) >> 4) | ((anim & 0x40) >> 6);
    const int dir = anim & 0x07;
    const float texanimxtab[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const float texanimytab[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];

    const float f = ( static_cast<int>( anim_time * (spd/15.0f) ) % 1600) / 1600.0f;
    glTranslatef(f*fdx, f*fdy, 0);
  }
}

void MapChunk::RemoveAnim (int anim) const
{
  if (anim) {
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glActiveTexture(GL_TEXTURE1);
  }
}


void MapChunk::drawTextures (int animation_time)
{
  glColor4f(1.0f,1.0f,1.0f,1.0f);

  if(nTextures > 0U)
  {
    opengl::texture::enable_texture (0);

    _textures[0]->bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    opengl::texture::disable_texture (1);
  }
  else
  {
    opengl::texture::disable_texture (0);
    opengl::texture::disable_texture (1);
  }

  SetAnim(animated[0], animation_time);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.0f,texDetail);
  glVertex3f(static_cast<float>(px), py+1.0f, -2.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(static_cast<float>(px), static_cast<float>(py), -2.0f);
  glTexCoord2f(texDetail, texDetail);
  glVertex3f(px+1.0f, py+1.0f, -2.0f);
  glTexCoord2f(texDetail, 0.0f);
  glVertex3f(px+1.0f, static_cast<float>(py), -2.0f);
  glEnd();
  RemoveAnim(animated[0]);

  if (nTextures > 1U) {
    //glDepthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    //glDepthMask(GL_FALSE);
  }
  for(size_t i=1; i < nTextures; ++i)
  {
    opengl::texture::enable_texture (0);

    _textures[i]->bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    opengl::texture::enable_texture (1);

    glBindTexture(GL_TEXTURE_2D, alphamaps[i-1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SetAnim(animated[i], animation_time);

    glBegin(GL_TRIANGLE_STRIP);
    glMultiTexCoord2f(GL_TEXTURE0, texDetail, 0.0f);
    glMultiTexCoord2f(GL_TEXTURE1, TEX_RANGE, 0.0f);
    glVertex3f(px+1.0f, static_cast<float>(py), -2.0f);
    glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
    glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
    glVertex3f(static_cast<float>(px), static_cast<float>(py), -2.0f);
    glMultiTexCoord2f(GL_TEXTURE0, texDetail, texDetail);
    glMultiTexCoord2f(GL_TEXTURE1, TEX_RANGE, TEX_RANGE);
    glVertex3f(px+1.0f, py+1.0f, -2.0f);
    glMultiTexCoord2f(GL_TEXTURE0, 0.0f, texDetail);
    glMultiTexCoord2f(GL_TEXTURE1, 0.0f, TEX_RANGE);
    glVertex3f(static_cast<float>(px), py+1.0f, -2.0f);
    glEnd();

    RemoveAnim(animated[i]);
  }

  opengl::texture::disable_texture (0);
  opengl::texture::disable_texture (1);

  glBindBuffer(GL_ARRAY_BUFFER, minimap);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, minishadows);
  glColorPointer(4, GL_FLOAT, 0, 0);

  glDrawElements(GL_TRIANGLE_STRIP, stripsize2, GL_UNSIGNED_SHORT, mapstrip2);
}

void MapChunk::initStrip()
{
    strip = new StripType[768]; //! \todo  figure out exact length of strip needed
    StripType* s = strip;
    for (size_t y=0; y < 8; ++y) {
      for (size_t x=0; x < 8; ++x) {
        if (!isHole(x, y)) {
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y, x); //0
            *s++ = indexNoLoD(y+1, x); //17
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y+1, x); //17
            *s++ = indexNoLoD(y+1, x+1); //18
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y+1, x+1); //18
            *s++ = indexNoLoD(y, x+1); //1
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y, x+1); //1
            *s++ = indexNoLoD(y, x); //0
        }
      }
    }
    striplen = static_cast<int>(s - strip);
}


MapChunk::~MapChunk()
{
  //! \todo random crash here.
  /*
    3   ???                                 0x0000000101930340 0x0 + 4321379136
    4   noggit                              0x00000001000298b8 _ZN8MapChunkD1Ev + 196
    5   noggit                              0x0000000100032ce4 _ZN7MapTileD1Ev + 266
    6   noggit                              0x00000001000bbadd _ZN5WorldD1Ev + 247
    7   noggit                              0x0000000100044500 _ZN7MapViewD0Ev + 106
    8   noggit                              0x000000010007cc66 SDL_main + 8004
   */

  // unload alpha maps
  glDeleteTextures( 3, alphamaps );
  // shadow maps, too
  glDeleteTextures( 1, &shadow );

  // delete VBOs
  glDeleteBuffers( 1, &vertices );
  glDeleteBuffers( 1, &normals );

  if( strip )
  {
    delete strip;
    strip = NULL;
  }

  if( nameID != -1 )
  {
    _world->selection_names().del( nameID );
    nameID = -1;
  }
}

bool MapChunk::GetVertex(float x,float z, ::math::vector_3d *V)
{
  float xdiff,zdiff;

  xdiff = x - xbase;
  zdiff = z - zbase;

  const int row = static_cast<int>( zdiff / (UNITSIZE * 0.5f ) + 0.5f );
  const int column = static_cast<int>( ( xdiff - UNITSIZE * 0.5f * (row % 2) ) / UNITSIZE + 0.5f );
  if( (row < 0) || (column < 0) || (row > 16) || (column > ((row % 2) ? 8 : 9)))
    return false;

  *V=mVertices[17*(row/2) + ((row % 2) ? 9 : 0) + column];
  return true;
}


void MapChunk::CreateStrips()
{
  StripType Temp[18];

  for(int i=0; i < 8; ++i)
  {
    _odd_strips[i*18+0] = i*17 + 17;
    for(int j=0; j < 8; j++)
    {
      _odd_strips[i*18 + 2*j + 1] = i*17 + j;
      _odd_strips[i*18 + 2*j + 2] = i*17 + j + 9;
      _even_strips[i*18 + 2*j] = i*17 + 17 + j;
      _even_strips[i*18 + 2*j + 1] = i*17 + 9 + j;
    }
    _odd_strips[i*18 + 17] = i*17 + 8;
    _even_strips[i*18 + 16] = i*17 + 17 + 8;
    _even_strips[i*18 + 17] = i*17 + 8;
  }

  //Reverse the order whoops
  for(int i=0; i < 8; ++i)
  {
    for(int j=0; j < 18; ++j)
      Temp[17-j] = _odd_strips[i*18 + j];
    memcpy(&_odd_strips[i*18], Temp, sizeof(Temp));
    for(int j=0; j < 18; ++j)
      Temp[17-j] = _even_strips[i*18 + j];
    memcpy(&_even_strips[i*18], Temp, sizeof(Temp));

  }

  for(int i=0; i < 32; ++i)
  {
    if(i < 9)
      _line_strip[i] = i;
    else if(i < 17)
      _line_strip[i] = 8 + (i-8)*17;
    else if(i < 25)
      _line_strip[i] = 145 - (i-15);
    else
      _line_strip[i] = (32-i)*17;
  }

  int iferget = 0;

  for( size_t i = 34; i < 43; ++i )
     _hole_strip[iferget++] = i;

  for( size_t i = 68; i < 77; ++i )
    _hole_strip[iferget++] = i;

  for( size_t i = 102; i < 111; ++i )
     _hole_strip[iferget++] = i;

  for( size_t i = 2; i < 139; i += 17 )
    _hole_strip[iferget++] = i;

  for( size_t i = 4; i < 141; i += 17 )
    _hole_strip[iferget++] = i;

  for( size_t i = 6; i < 143; i += 17 )
    _hole_strip[iferget++] = i;
}

void MapChunk::drawColor (bool draw_fog)
{

  if (!_world->frustum.intersects(vmin,vmax))
    return;

  float mydist = (_world->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  if (mydist > _world->culldistance)
  {
    if (draw_fog)
      drawNoDetail();
    return;
  }

  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);

  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  //glDisable(GL_LIGHTING);

  glBegin(GL_TRIANGLE_STRIP);
  for(int i=0; i < striplen; ++i)
  {
    ::math::vector_3d Color;
    HeightColor( mVertices[strip[i]].y(), &Color);
    glColor3fv(Color);
    glNormal3fv(mNormals[strip[i]]);
    glVertex3fv(mVertices[strip[i]]);
  }
  glEnd();
  //glEnable(GL_LIGHTING);
}


void MapChunk::drawPass (int anim, int animation_time)
{
  if (anim)
  {
    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();

    // note: this is ad hoc and probably completely wrong
    const int spd = (anim & 0x08) | ((anim & 0x10) >> 2) | ((anim & 0x20) >> 4) | ((anim & 0x40) >> 6);
    const int dir = anim & 0x07;
    const float texanimxtab[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const float texanimytab[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];
    const int animspd = 200 * detail_size;
    float f = ( (static_cast<int>(animation_time*(spd/15.0f))) % animspd) / static_cast<float>(animspd);
    glTranslatef(f*fdx,f*fdy,0);
  }

  glDrawElements(GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, strip);

  if (anim)
  {
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glActiveTexture(GL_TEXTURE1);
  }
}

void MapChunk::drawLines (bool draw_hole_lines)
{
  if (!_world->frustum.intersects(vmin,vmax))
    return;

  float mydist = (_world->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  glBindBuffer(GL_ARRAY_BUFFER, vertices);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  glPushMatrix();
  glColor4f(1.0,0.0,0.0f,0.5f);
  glTranslatef(0.0f,0.05f,0.0f);
  glEnable (GL_LINE_SMOOTH);
  glLineWidth(1.5);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

  if( (px != 15) && (py != 0))
  {
    glDrawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, _line_strip);
  }
  else if( (px==15) && (py==0) )
  {
    glColor4f(0.0,1.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, _line_strip);
  }
  else if(px==15)
  {
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, _line_strip);
    glColor4f(0.0,1.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_line_strip[8]);
  }
  else if(py==0)
  {
    glColor4f(0.0,1.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, _line_strip);
    glColor4f(1.0,0.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_line_strip[8]);
  }

  if(draw_hole_lines)
  {
    glColor4f(0.0,0.0,1.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, _hole_strip);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[9]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[18]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[27]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[36]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[45]);
  }

  glPopMatrix();
  glEnable(GL_LIGHTING);
  glColor4f(1,1,1,1);
}

void MapChunk::drawContour()
{
  glColor4f(1,1,1,1);
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  if(_contour_texture == 0)
    GenerateContourMap();
  glBindTexture(GL_TEXTURE_2D, _contour_texture);

  glEnable(GL_TEXTURE_GEN_S);
  glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  glTexGenfv(GL_S,GL_OBJECT_PLANE,_contour_coord_gen);

  drawPass(0);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
}



void MapChunk::draw ( bool draw_terrain_height_contour
                    , bool mark_impassable_chunks
                    , bool draw_area_id_overlay
                    , bool dont_draw_cursor
                    )
{

  if (!_world->frustum.intersects( vmin, vmax ))
    return;

  float mydist = (_world->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  // setup vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, vertices);
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, normals);
  glNormalPointer(GL_FLOAT, 0, 0);
  // ASSUME: texture coordinates set up already


  // first pass: base texture
  if (nTextures == 0U)
  {
    opengl::texture::disable_texture (0);
    opengl::texture::disable_texture (1);

    glColor3f(1.0f,1.0f,1.0f);
  }
  else
  {
    opengl::texture::enable_texture (0);

    _textures[0]->bind();

    opengl::texture::disable_texture (1);
  }

  glEnable(GL_LIGHTING);
  drawPass(animated[0], _world->animtime);

  if (nTextures > 1U) {
    //glDepthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    glDepthMask(GL_FALSE);
  }

  // additional passes: if required
  for( size_t i = 1; i < nTextures; ++i )
  {
    opengl::texture::enable_texture (0);

    _textures[i]->bind();

    // this time, use blending:
    opengl::texture::enable_texture (1);

    glBindTexture( GL_TEXTURE_2D, alphamaps[i - 1] );

    drawPass(animated[i], _world->animtime);
  }

  if (nTextures > 1U) {
    //glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
  }

  // shadow map
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  ::math::vector_3d shc = _world->skies->colorSet[WATER_COLOR_DARK] * 0.3f;
  glColor4f(shc.x(),shc.y(),shc.z(),1);

  //glColor4f(1,1,1,1);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, shadow);
  glEnable(GL_TEXTURE_2D);

  drawPass (0);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  if (draw_terrain_height_contour)
  {
    drawContour();
  }

  if (mark_impassable_chunks && Flags & FLAG_IMPASS)
  {
    glColor4f (1.0f, 1.0f, 1.0f, 0.6f);
    drawPass (0);
  }

  if (draw_area_id_overlay)
  {
    glColor4f ( areaIDColors[areaID].x()
              , areaIDColors[areaID].y()
              , areaIDColors[areaID].z()
              , 0.7f
              );
    drawPass (0);
  }

  //! \todo This actually should be an enum. And should be passed into this method.
  if ( QSettings().value ("cursor/type", 1).toInt() == 3
    && _world->IsSelection (eEntry_MapChunk)
    && _world->GetCurrentSelection()->data.mapchunk == this
    && !dont_draw_cursor
     )
  {
    const int poly (_world->GetCurrentSelectedTriangle());

    glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

    glPushMatrix();

    glDisable( GL_CULL_FACE );
    glDepthMask( false );
    glDisable( GL_DEPTH_TEST );
    glBegin( GL_TRIANGLES );
    glVertex3fv( mVertices[mapstrip2[poly + 0]] );
    glVertex3fv( mVertices[mapstrip2[poly + 1]] );
    glVertex3fv( mVertices[mapstrip2[poly + 2]] );
    glEnd();
    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
    glDepthMask( true );

    glPopMatrix();
  }


  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

  glEnable( GL_LIGHTING );
  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

  /*
  //////////////////////////////////
  // debugging tile flags:
  GLfloat tcols[8][4] = {  {1,1,1,1},
    {1,0,0,1}, {1, 0.5f, 0, 1}, {1, 1, 0, 1},
    {0,1,0,1}, {0,1,1,1}, {0,0,1,1}, {0.8f, 0, 1,1}
  };
  glPushMatrix();
  glDisable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
  glTranslatef(xbase, ybase, zbase);
  for (int i=0; i<8; ++i) {
    int v = 1 << (7-i);
    for (int j=0; j<4; j++) {
      if (animated[j] & v) {
        glBegin(GL_TRIANGLES);
        glColor4fv(tcols[i]);

        glVertex3f(i*2.0f, 2.0f, j*2.0f);
        glVertex3f(i*2.0f+1.0f, 2.0f, j*2.0f);
        glVertex3f(i*2.0f+0.5f, 4.0f, j*2.0f);

        glEnd();
      }
    }
  }
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);
  glColor4f(1,1,1,1);
  glPopMatrix();*/
}

void MapChunk::drawNoDetail()
{
  glActiveTexture( GL_TEXTURE1 );
  glDisable( GL_TEXTURE_2D );
  glActiveTexture(GL_TEXTURE0 );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_LIGHTING );

  //glColor3fv(_world->skies->colorSet[FOG_COLOR]);
  //glColor3f(1,0,0);
  //glDisable(GL_FOG);

  // low detail version
  glBindBuffer( GL_ARRAY_BUFFER, vertices );
  glVertexPointer( 3, GL_FLOAT, 0, 0 );
  glDisableClientState( GL_NORMAL_ARRAY );
  glDrawElements( GL_TRIANGLE_STRIP, stripsize, GL_UNSIGNED_SHORT, mapstrip );
  glEnableClientState( GL_NORMAL_ARRAY );

  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
  //glEnable(GL_FOG);

  glEnable( GL_LIGHTING );
  glActiveTexture( GL_TEXTURE1 );
  glEnable( GL_TEXTURE_2D );
  glActiveTexture( GL_TEXTURE0 );
  glEnable( GL_TEXTURE_2D );
}

void MapChunk::drawSelect()
{
  if( !_world->frustum.intersects( vmin, vmax ) )
    return;

  float mydist = (_world->camera - vcenter).length() - r;
  if (mydist > (mapdrawdistance * mapdrawdistance)) return;
  if (mydist > _world->culldistance)
    return;

  if( nameID == -1 )
    nameID = _world->selection_names().add( this );

  //! \todo Use backface culling again? Maybe this adds problems. Idk.
  //glDisable( GL_CULL_FACE );
  glPushName( nameID );

  for( int i = 0; i < stripsize2 - 2; ++i )
  {
    glPushName( i );
    glBegin( GL_TRIANGLES );
    glVertex3fv( mVertices[mapstrip2[i]] );
    glVertex3fv( mVertices[mapstrip2[i + 1]] );
    glVertex3fv( mVertices[mapstrip2[i + 2]] );
    glEnd();
    glPopName();
  }

  glPopName();
  //glEnable( GL_CULL_FACE );
}

void MapChunk::getSelectionCoord( float *x, float *z )
{
  int Poly = _world->GetCurrentSelectedTriangle();
  if( Poly + 2 > stripsize2 )
  {
    *x = -1000000.0f;
    *z = -1000000.0f;
    return;
  }
  *x = ( mVertices[mapstrip2[Poly + 0]].x() + mVertices[mapstrip2[Poly + 1]].x() + mVertices[mapstrip2[Poly + 2]].x() ) / 3;
  *z = ( mVertices[mapstrip2[Poly + 0]].z() + mVertices[mapstrip2[Poly + 1]].z() + mVertices[mapstrip2[Poly + 2]].z() ) / 3;
}

float MapChunk::getSelectionHeight()
{
  int Poly = _world->GetCurrentSelectedTriangle();
  if( Poly + 2 < stripsize2 )
    return ( mVertices[mapstrip2[Poly + 0]].y() + mVertices[mapstrip2[Poly + 1]].y() + mVertices[mapstrip2[Poly + 2]].y() ) / 3;
  LogError << "Getting selection height fucked up because the selection was bad. " << Poly << "%i with striplen of " << stripsize2 << "." << std::endl;
  return 0.0f;
}

::math::vector_3d MapChunk::GetSelectionPosition()
{
  int Poly = _world->GetCurrentSelectedTriangle();
  if( Poly + 2 > stripsize2 )
  {
    LogError << "Getting selection position fucked up because the selection was bad. " << Poly << "%i with striplen of " << stripsize2 << "." << std::endl;
    return ::math::vector_3d( -1000000.0f, -1000000.0f, -1000000.0f );
  }

  ::math::vector_3d lPosition;
  lPosition  = ::math::vector_3d( mVertices[mapstrip2[Poly + 0]] );
  lPosition += ::math::vector_3d( mVertices[mapstrip2[Poly + 1]] );
  lPosition += ::math::vector_3d( mVertices[mapstrip2[Poly + 2]] );
  lPosition *= 0.3333333f;

  return lPosition;
}

void MapChunk::recalcNorms()
{

  ::math::vector_3d P1,P2,P3,P4;
  ::math::vector_3d Norm,N1,N2,N3,N4,D;


  if(Changed==false)
    return;
  Changed=false;

  for(int i=0;i<mapbufsize;++i)
  {
    if(!_world->GetVertex( mVertices[i].x() - UNITSIZE*0.5f, mVertices[i].z() - UNITSIZE*0.5f, &P1 ))
    {
      P1.x (mVertices[i].x() - UNITSIZE*0.5f);
      P1.y (mVertices[i].y());
      P1.z (mVertices[i].z() - UNITSIZE*0.5f);
    }

    if(!_world->GetVertex( mVertices[i].x() + UNITSIZE*0.5f, mVertices[i].z() - UNITSIZE*0.5f, &P2 ))
    {
      P2.x (mVertices[i].x() + UNITSIZE*0.5f);
      P2.y (mVertices[i].y());
      P2.z (mVertices[i].z() - UNITSIZE*0.5f);
    }

    if(!_world->GetVertex( mVertices[i].x() + UNITSIZE*0.5f, mVertices[i].z() + UNITSIZE*0.5f, &P3 ))
    {
      P3.x (mVertices[i].x() + UNITSIZE*0.5f);
      P3.y (mVertices[i].y());
      P3.z (mVertices[i].z() + UNITSIZE*0.5f);
    }

    if(!_world->GetVertex( mVertices[i].x() - UNITSIZE*0.5f, mVertices[i].z() + UNITSIZE*0.5f, &P4 ))
    {
      P4.x (mVertices[i].x() - UNITSIZE*0.5f);
      P4.y (mVertices[i].y());
      P4.z (mVertices[i].z() + UNITSIZE*0.5f);
    }

    N1 = (P2 - mVertices[i]) % (P1 - mVertices[i]);
    N2 = (P3 - mVertices[i]) % (P2 - mVertices[i]);
    N3 = (P4 - mVertices[i]) % (P3 - mVertices[i]);
    N4 = (P1 - mVertices[i]) % (P4 - mVertices[i]);

    Norm = N1 + N2 + N3 + N4;
    Norm.normalize();
    mNormals[i] = Norm;
  }
  glBindBuffer(GL_ARRAY_BUFFER, normals);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mNormals), mNormals, GL_STATIC_DRAW);

  for (size_t j (0); j < mapbufsize; ++j)
  {
    mFakeShadows[j].x (0.0f);
    mFakeShadows[j].y (0.0f);
    mFakeShadows[j].z (0.0f);
    mFakeShadows[j].w ( qBound ( 0.0f
                               , 1.0f - (-mNormals[j].x()
                                        + mNormals[j].y()
                                        - mNormals[j].z()
                                        )
                               , 1.0f
                               )
                      * 0.5f
                      );
  }

  glBindBuffer(GL_ARRAY_BUFFER, minishadows);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

bool MapChunk::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  float dist,xdiff,zdiff;

  Changed=false;

  xdiff = xbase - x + CHUNKSIZE/2;
  zdiff = zbase - z + CHUNKSIZE/2;
  dist = sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + MAPCHUNK_DIAMETER))
    return Changed;
  vmin.y (9999999.0f);
  vmax.y (-9999999.0f);
  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x() - x;
    zdiff = mVertices[i].z() - z;
    if(BrushType == 5){
      if((abs(xdiff) < abs(radius/2)) && (abs(zdiff) < abs(radius/2))){
        mVertices[i].y (mVertices[i].y() + change);
        Changed=true;
      }
    }
    else
    {
      dist = sqrt(xdiff*xdiff + zdiff*zdiff);
      if(dist < radius)
      {

        if(BrushType==0)//Flat
          mVertices[i].y (mVertices[i].y() + change);

        else if(BrushType==1)//Linear
          mVertices[i].y (mVertices[i].y() + change*(1.0f - dist/radius));

        else if(BrushType==2)//Smooth
          mVertices[i].y (mVertices[i].y() + change/(1.0f + dist/radius));

        else if (BrushType == 3) //x^2
          mVertices[i].y (mVertices[i].y() + change*( (dist/radius)*(dist/radius) + dist/radius + 1.0f));

        else if (BrushType == 4) //cos
          mVertices[i].y (mVertices[i].y() + change*cos(dist/radius));

        Changed=true;
      }
    }

    vmin.y (std::min(vmin.y(), mVertices[i].y()));
    vmax.y (std::max(vmax.y(), mVertices[i].y()));
  }
  if(Changed)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}


bool MapChunk::flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType)
{
  float dist,xdiff,zdiff,nremain;
  Changed=false;

  xdiff= xbase - x + CHUNKSIZE/2;
  zdiff= zbase - z + CHUNKSIZE/2;
  dist= sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + MAPCHUNK_DIAMETER))
    return Changed;

  vmin.y (9999999.0f);
  vmax.y (-9999999.0f);

  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x() - x;
    zdiff = mVertices[i].z() - z;

    dist=sqrt(xdiff*xdiff + zdiff*zdiff);

    if(dist < radius)
    {
      if(BrushType==0)//Flat
      {
        mVertices[i].y (remain*mVertices[i].y() + (1 - remain)*h);
      }
      else if(BrushType==1)//Linear
      {
        nremain = 1 - (1 - remain) * (1 - dist/radius);
        mVertices[i].y (nremain*mVertices[i].y() + (1-nremain)*h);
      }
      else if(BrushType==2)//Smooth
      {
        nremain = 1.0f - pow(1.0f - remain, (1.0f + dist/radius));
        mVertices[i].y (nremain*mVertices[i].y() + (1 - nremain)*h);
      }

      Changed=true;
    }

    vmin.y (std::min(vmin.y(), mVertices[i].y()));
    vmax.y (std::max(vmax.y(), mVertices[i].y()));
  }
  if(Changed)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}

bool MapChunk::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
  float dist,dist2,xdiff,zdiff,nremain;
  Changed = false;

  xdiff = xbase - x + CHUNKSIZE/2;
  zdiff = zbase - z + CHUNKSIZE/2;
  dist = sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + MAPCHUNK_DIAMETER) )
    return Changed;

  vmin.y (9999999.0f);
  vmax.y (-9999999.0f);

  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff= mVertices[i].x() - x;
    zdiff= mVertices[i].z() - z;

    dist= sqrt(xdiff*xdiff + zdiff*zdiff);

    if(dist < radius)
    {
      float TotalHeight;
      float TotalWeight;
      float tx,tz, h;
      ::math::vector_3d TempVec;
      int Rad=(radius/UNITSIZE);

      TotalHeight=0;
      TotalWeight=0;
      for(int j= -Rad*2; j <= Rad*2; ++j)
      {
        tz= z + j * UNITSIZE/2;
        for(int k=-Rad; k <= Rad; ++k)
        {
          tx= x + k*UNITSIZE + (j%2) * UNITSIZE/2.0f;
          xdiff= tx - mVertices[i].x();
          zdiff= tz - mVertices[i].z();
          dist2= sqrt(xdiff*xdiff + zdiff*zdiff);
          if(dist2 > radius)
            continue;
          _world->GetVertex(tx,tz,&TempVec);
          TotalHeight += (1.0f - dist2/radius) * TempVec.y();
          TotalWeight += (1.0f - dist2/radius);
        }
      }

      h=TotalHeight/TotalWeight;

      if(BrushType==0)//Flat
      {
        mVertices[i].y (remain * mVertices[i].y() + (1 - remain) * h);
      }
      else if(BrushType==1)//Linear
      {
        nremain= 1 - (1 - remain) * (1 - dist/radius);
        mVertices[i].y (nremain * mVertices[i].y() + ( 1 - nremain) * h);
      }
      else if(BrushType==2)//Smooth
      {
        nremain= 1.0f - pow( 1.0f - remain , (1.0f + dist/radius) );
        mVertices[i].y (nremain*mVertices[i].y() + (1-nremain)*h);
      }

      Changed=true;
    }

    vmin.y (std::min(vmin.y(), mVertices[i].y()));
    vmax.y (std::max(vmax.y(), mVertices[i].y()));
  }
  if(Changed)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}

/* The correct way to do everything
Visibility = (1-Alpha above)*Alpha

Objective is Visibility = level

if (not bottom texture)
  New Alpha = Pressure*Level+(1-Pressure)*Alpha;
  New Alpha Above = (1-Pressure)*Alpha Above;
else Bottom Texture
  New Alpha Above = Pressure*(1-Level)+(1-Pressure)*Alpha Above

For bottom texture with multiple above textures

For 2 textures above
x,y = current alphas
u,v = target alphas
v=sqrt((1-level)*y/x)
u=(1-level)/v

For 3 textures above
x,y,z = current alphas
u,v,w = target alphas
L=(1-Level)
u=pow(L*x*x/(y*y),1.0f/3.0f)
w=sqrt(L*z/(u*y))
*/
void MapChunk::eraseTextures()
{
  nTextures = 0U;
}

int MapChunk::addTexture( noggit::blp_texture* texture )
{
  int texLevel = -1;
  if( nTextures < 4U )
  {
    texLevel = nTextures;
    nTextures++;
    _textures[texLevel] = texture;
    animated[texLevel] = 0;
    texFlags[texLevel] = 0;
    effectID[texLevel] = 0;
    if( texLevel )
    {
      if( alphamaps[texLevel-1] < 1 )
      {
        LogError << "Alpha Map has invalid texture binding" << std::endl;
        nTextures--;
        return -1;
      }
      memset( amap[texLevel - 1], 0, 64 * 64 );
      glBindTexture( GL_TEXTURE_2D, alphamaps[texLevel - 1] );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[texLevel - 1] );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }
  }
  return texLevel;
}
void MapChunk::switchTexture( noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture )
{
  int texLevel = -1;
  for(size_t i=0;i<nTextures;++i)
    if(_textures[i]==oldTexture)
      texLevel=i;

  if(texLevel != -1)
  {
  _textures[texLevel] = newTexture;
  }
}
bool MapChunk::paintTexture( float x, float z, const brush& Brush, float strength, float pressure, noggit::blp_texture* texture )
{
#if 1
  float zPos,xPos,change,xdiff,zdiff,dist, radius;

  int texLevel=-1;

  radius=Brush.getRadius();

  xdiff= xbase - x + CHUNKSIZE/2;
  zdiff= zbase - z + CHUNKSIZE/2;
  dist= sqrt( xdiff*xdiff + zdiff*zdiff );

  if( dist > (radius+MAPCHUNK_DIAMETER) )
    return false;

  //First Lets find out do we have the texture already
  for(size_t i=0;i<nTextures;++i)
    if(_textures[i]==texture)
      texLevel=i;


  if( (texLevel==-1) && (nTextures==4) )
  {
    // Implement here auto texture slot freeing :)
    LogDebug << "paintTexture: No free texture slot" << std::endl;
    return false;
  }

  //Only 1 layer and its that layer
  if( (texLevel!=-1) && (nTextures==1) )
    return true;


  change=CHUNKSIZE/62.0f;
  zPos=zbase;

  float target,tarAbove, tPressure;
  //int texAbove=nTextures-texLevel-1;


  for(int j=0; j < 63 ; j++)
  {
    xPos=xbase;
    for(int i=0; i < 63; ++i)
    {
      xdiff=xPos-x;
      zdiff=zPos-z;
      dist=abs(sqrt( xdiff*xdiff + zdiff*zdiff ));

      if(dist>radius)
      {
        xPos+=change;
        continue;
      }

      if(texLevel==-1)
      {
        texLevel=addTexture(texture);
        if(texLevel==0)
          return true;
        if(texLevel==-1)
        {
          LogDebug << "paintTexture: Unable to add texture." << std::endl;
          return false;
        }
      }

      target=strength;
      tarAbove=1-target;

      tPressure=pressure*Brush.getValue(dist);

      if(texLevel>0)
      {
        uchar test = static_cast<unsigned char>(std::max( std::min( (1.0f-tPressure)*( static_cast<float>(amap[texLevel-1][i+j*64]) ) + tPressure*target + 0.5f ,255.0f) , 0.0f));
        amap[texLevel-1][i+j*64]=test;
      }
      for(size_t k=texLevel;k<nTextures-1;k++)
        amap[k][i+j*64]=static_cast<unsigned char>(std::max( std::min( (1.0f-tPressure)*( static_cast<float>(amap[k][i+j*64]) ) + tPressure*tarAbove + 0.5f ,255.0f) , 0.0f));
      xPos+=change;
    }
    zPos+=change;
  }

  if( texLevel == -1 )
  {
    LogDebug << "Somehow no texture got painted." << std::endl;
    return false;
  }

  for( size_t j = texLevel; j < nTextures - 1; j++ )
  {
    if( j > 2 )
    {
      LogError << "WTF how did you get here??? Get a cookie." << std::endl;
      continue;
    }
    glBindTexture( GL_TEXTURE_2D, alphamaps[j] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j] );
  }

  if( texLevel )
  {
    glBindTexture( GL_TEXTURE_2D, alphamaps[texLevel - 1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[texLevel - 1] );
  }
#else
  // new stuff from bernd.
  // need to get rework. Add old code with switch that the guys out there can use paint.
  const float radius = Brush.getRadius();

  // Are we really painting on this chunk?
  const float xdiff = xbase + CHUNKSIZE / 2 - x;
  const float zdiff = zbase + CHUNKSIZE / 2 - z;

  if( ( xdiff * xdiff + zdiff * zdiff ) > ( MAPCHUNK_DIAMETER / 2 + radius ) * ( MAPCHUNK_DIAMETER / 2 + radius ) )
  return false;


  // Search for empty layer.
  int texLevel = -1;

  for( size_t i = 0; i < nTextures; ++i )
  {
    if( _textures[i] == texture )
    {
      texLevel = i;
    }
   }

  if( texLevel == -1 )
  {

    if( nTextures == 4 )
    {
      for( size_t layer = 0; layer < nTextures; ++layer )
      {
        unsigned char map[64*64];
        if( layer )
          memcpy( map, amap[layer-1], 64*64 );
        else
          memset( map, 255, 64*64 );

        for( size_t layerAbove = layer + 1; layerAbove < nTextures; ++layerAbove )
        {
          unsigned char* above = amap[layerAbove-1];
          for( size_t i = 0; i < 64 * 64; ++i )
          {
            map[i] = std::max( 0, map[i] - above[i] );
          }
        }

        size_t sum = 0;
        for( size_t i = 0; i < 64 * 64; ++i )
        {
          sum += map[i];
        }

        if( !sum )
        {
          for( size_t i = layer; i < nTextures - 1; ++i )
          {
            _textures[i] = _textures[i+1];
            animated[i] = animated[i+1];
            texFlags[i] = texFlags[i+1];
            effectID[i] = effectID[i+1];
            if( i )
              memcpy( amap[i-1], amap[i], 64*64 );
          }

          for( size_t j = layer; j < nTextures; j++ )
              {
                glBindTexture( GL_TEXTURE_2D, alphamaps[j - 1] );
                glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1] );
              }
              nTextures--;
        }
      }
    }

    if( nTextures == 4 )
    return false;

      texLevel = addTexture( texture );

  }
  else
  {
    if( nTextures == 1 )
      return true;
  }
  LogDebug << "TexLevel: " << texLevel << " -  NTextures: " << nTextures << "\n";
  // We now have a texture at texLevel > 0.
  static const float change = CHUNKSIZE / 62.0f; //! \todo 64? 63? 62? Wtf?

  if( texLevel == 0 )
    return true;

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      const float xdiff_ = xbase + change * i - x;
      const float zdiff_ = zbase + change * j - z;
      const float dist = sqrtf( xdiff_ * xdiff_ + zdiff_ * zdiff_ );

      if( dist <= radius )
      {
          amap[texLevel - 1][i + j * 64] = (unsigned char)( std::max( std::min( amap[texLevel - 1][i + j * 64] + pressure * strength * Brush.getValue( dist ) + 0.5f, 255.0f ), 0.0f ) );
      }
    }
  }


  // Redraw changed layers.

  for( size_t j = texLevel; j < nTextures; j++ )
  {
    glBindTexture( GL_TEXTURE_2D, alphamaps[j - 1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1] );
  }
#endif

  return true;
}

bool MapChunk::isHole( int i, int j )
{
  return( holes & ( ( 1 << ((j*4)+i) ) ));
}

void MapChunk::addHole( int i, int j )
{
  holes = holes | ( ( 1 << ((j*4)+i)) );
  initStrip();
}

void MapChunk::removeHole( int i, int j )
{
  holes = holes & ~( ( 1 << ((j*4)+i)) );
  initStrip();
}

void MapChunk::setAreaID( int ID )
{
  areaID = ID;
}

int MapChunk::getAreaID(){
  return areaID;
}


void MapChunk::setFlag( bool on_or_off, int flag)
{
  if(on_or_off)
    Flags = Flags | flag;
  else
    Flags = Flags & ~(flag);
}
