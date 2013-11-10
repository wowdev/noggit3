#include "MapChunk.h"

#include <algorithm>
#include <iostream>
#include <map>

#include "Brush.h"
#include "Environment.h"
#include "Liquid.h"
#include "Log.h"
#include "MapHeaders.h"
#include "Misc.h"
#include "Quaternion.h"
#include "TextureSet.h"
#include "Vec3D.h"
#include "World.h"
#include "Alphamap.h"

extern int terrainMode;

static const int HEIGHT_TOP = 1000;
static const int HEIGHT_MID = 600;
static const int HEIGHT_LOW = 300;
static const int HEIGHT_ZERO = 0;
static const int HEIGHT_SHALLOW = -100;
static const int HEIGHT_DEEP = -250;

bool drawFlags = false;
bool DrawMapContour = false;

GLuint Contour = 0;
float CoordGen[4];
static const int CONTOUR_WIDTH = 128;

static const float texDetail = 8.0f;

static const float TEX_RANGE = 62.0f / 64.0f;

StripType OddStrips[8*18];
StripType EvenStrips[8*18];
StripType LineStrip[32];
StripType HoleStrip[128];

void GenerateContourMap()
{
  unsigned char  CTexture[CONTOUR_WIDTH*4];

  CoordGen[0]=0.0f;
  CoordGen[1]=0.25f;
  CoordGen[2]=0.0f;
  CoordGen[3]=0.0f;


  for(int i=0;i<(CONTOUR_WIDTH*4);++i)
    CTexture[i]=0;
  CTexture[3+CONTOUR_WIDTH/2]=0xff;
  CTexture[7+CONTOUR_WIDTH/2]=0xff;
  CTexture[11+CONTOUR_WIDTH/2]=0xff;

  glGenTextures(1, &Contour);
  glBindTexture(GL_TEXTURE_2D, Contour);


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
  glTexGenfv(GL_S,GL_OBJECT_PLANE,CoordGen);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

void CreateStrips()
{
  StripType Temp[18];
  int j;

  for(int i=0; i < 8; ++i)
  {
    OddStrips[i*18+0] = i*17 + 17;
    for(j=0; j < 8; j++)
    {
      OddStrips[i*18 + 2*j + 1] = i*17 + j;
      OddStrips[i*18 + 2*j + 2] = i*17 + j + 9;
      EvenStrips[i*18 + 2*j] = i*17 + 17 + j;
      EvenStrips[i*18 + 2*j + 1] = i*17 + 9 + j;
    }
    OddStrips[i*18 + 17] = i*17 + 8;
    EvenStrips[i*18 + 16] = i*17 + 17 + 8;
    EvenStrips[i*18 + 17] = i*17 + 8;
  }

  //Reverse the order whoops
  for(int i=0; i < 8; ++i)
  {
    for(j=0; j < 18; ++j)
      Temp[17-j] = OddStrips[i*18 + j];
    memcpy(&OddStrips[i*18], Temp, sizeof(Temp));
    for(j=0; j < 18; ++j)
      Temp[17-j] = EvenStrips[i*18 + j];
    memcpy(&EvenStrips[i*18], Temp, sizeof(Temp));

  }

  for(int i=0; i < 32; ++i)
  {
    if(i < 9)
      LineStrip[i] = i;
    else if(i < 17)
      LineStrip[i] = 8 + (i-8)*17;
    else if(i < 25)
      LineStrip[i] = 145 - (i-15);
    else
      LineStrip[i] = (32-i)*17;
  }

  int iferget = 0;

  for( size_t i = 34; i < 43; ++i )
    HoleStrip[iferget++] = i;

  for( size_t i = 68; i < 77; ++i )
    HoleStrip[iferget++] = i;

  for( size_t i = 102; i < 111; ++i )
    HoleStrip[iferget++] = i;

  for( size_t i = 2; i < 139; i += 17 )
    HoleStrip[iferget++] = i;

  for( size_t i = 4; i < 141; i += 17 )
    HoleStrip[iferget++] = i;

  for( size_t i = 6; i < 143; i += 17 )
    HoleStrip[iferget++] = i;
}



MapChunk::MapChunk(MapTile* maintile, MPQFile* f,bool bigAlpha)
  : textureSet(new TextureSet)
{
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

  if( Environment::getInstance()->areaIDColors.find(areaID) == Environment::getInstance()->areaIDColors.end() )
  {
    Vec3D newColor = Vec3D( misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) );
    Environment::getInstance()->areaIDColors.insert( std::pair<int,Vec3D>(areaID, newColor) );
  }

  Environment::getInstance()->selectedAreaID = areaID; //The last loaded is selected on start.

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

  vmin = Vec3D( 9999999.0f, 9999999.0f, 9999999.0f);
  vmax = Vec3D(-9999999.0f,-9999999.0f,-9999999.0f);

  while (f->getPos() < lastpos) {
    f->read(&fourcc,4);
    f->read(&size, 4);

    size_t nextpos = f->getPos() + size;

    if ( fourcc == 'MCNR' ) {
      nextpos = f->getPos() + 0x1C0; // size fix
      // normal vectors
      char nor[3];
      Vec3D *ttn = mNormals;
      for (int j=0; j<17; ++j) {
        for (int i=0; i<((j%2)?8:9); ++i) {
          f->read(nor,3);
          // order X,Z,Y
          // *ttn++ = Vec3D((float)nor[0]/127.0f, (float)nor[2]/127.0f, (float)nor[1]/127.0f);
          *ttn++ = Vec3D(-nor[1]/127.0f, nor[2]/127.0f, -nor[0]/127.0f);
        }
      }
    }
    else if ( fourcc == 'MCVT' ) {
      Vec3D *ttv = mVertices;

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
          Vec3D v = Vec3D(xbase+xpos, ybase+h, zbase+zpos);
          *ttv++ = v;
          vmin.y = std::min(vmin.y, v.y);
          vmax.y = std::max(vmax.y, v.y);
        }
      }

      vmin.x = xbase;
      vmin.z = zbase;
      vmax.x = xbase + 8 * UNITSIZE;
      vmax.z = zbase + 8 * UNITSIZE;
      r = (vmax - vmin).length() * 0.5f;

    }
    else if ( fourcc == 'MCLY' )
    {
      textureSet->initTextures(f, mt, size);
    }
    else if ( fourcc == 'MCSH' )
    {
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
      textureSet->initAlphamaps(f, header.nLayers, mBigAlpha);
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

  this->mt = mt;

  vcenter = (vmin + vmax) * 0.5f;

  nameID = SelectionNames.add( this );



  Vec3D *ttv = mMinimap;

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
      Vec3D v = Vec3D(xpos+px, zpos+py, -1);
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

  float ShadowAmount;
  for (int j=0; j<mapbufsize;++j)
  {
    //tm[j].z=tv[j].y;
    ShadowAmount=1.0f-(-mNormals[j].x+mNormals[j].y-mNormals[j].z);
    if(ShadowAmount<0)
      ShadowAmount=0.0f;
    if(ShadowAmount>1.0)
      ShadowAmount=1.0f;
    ShadowAmount*=0.5f;
    //ShadowAmount=0.2;
    mFakeShadows[j].x=0;
    mFakeShadows[j].y=0;
    mFakeShadows[j].z=0;
    mFakeShadows[j].w=ShadowAmount;
  }

  glGenBuffers(1,&minimap);
  glGenBuffers(1,&minishadows);

  glBindBuffer(GL_ARRAY_BUFFER, minimap);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mMinimap), mMinimap, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, minishadows);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

void MapChunk::drawTextures()
{

  glColor4f(1.0f,1.0f,1.0f,1.0f);

  if(textureSet->num() > 0U)
  {
    textureSet->bindTexture(0, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    OpenGL::Texture::setActiveTexture( 1 );
    OpenGL::Texture::disableTexture();
  }
  else
  {
    OpenGL::Texture::setActiveTexture( 0 );
    OpenGL::Texture::disableTexture();

    OpenGL::Texture::setActiveTexture( 1 );
    OpenGL::Texture::disableTexture();
  }

  textureSet->start2DAnim(0);
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
  textureSet->stop2DAnim(0);

  if (textureSet->num() > 1U)
  {
    //glDepthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    //glDepthMask(GL_FALSE);
  }

  for(size_t i=1; i < textureSet->num(); ++i)
  {
    textureSet->bindTexture(i, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    textureSet->bindAlphamap(i-1, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    textureSet->start2DAnim(i);

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

    textureSet->start2DAnim(i);
  }

  OpenGL::Texture::setActiveTexture( 0 );
  OpenGL::Texture::disableTexture();

  OpenGL::Texture::setActiveTexture( 1 );
  OpenGL::Texture::disableTexture();

  glBindBuffer(GL_ARRAY_BUFFER, minimap);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, minishadows);
  glColorPointer(4, GL_FLOAT, 0, 0);

  glDrawElements(GL_TRIANGLE_STRIP, stripsize2, GL_UNSIGNED_SHORT, gWorld->mapstrip2);
}

int MapChunk::indexLoD(int x, int y)
{
  return (x+1)*9+x*8+y;
}

int MapChunk::indexNoLoD(int x, int y)
{
  return x*8+x*9+y;
}

void MapChunk::initStrip()
{
  strip = new StripType[768]; //! \todo  figure out exact length of strip needed
  StripType* s = strip;

  for(int x=0; x<8; ++x)
  {
    for(int y=0; y<8; ++y)
    {
      if (isHole(x/2, y/2))
        continue;

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

  delete textureSet;

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
    SelectionNames.del( nameID );
    nameID = -1;
  }
}

bool MapChunk::GetVertex(float x,float z, Vec3D *V)
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

float MapChunk::getHeight(int x, int z)
{
  if(x > 9 || z > 9 || x < 0 || z < 0) return 0.0f;
  return mVertices[indexNoLoD(x, z)].y;
}

void MapChunk::drawPass(int id)
{
  textureSet->startAnim(id);
  glDrawElements(GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, strip);
  textureSet->stopAnim(id);
}

void MapChunk::drawLines()
{
  if (!gWorld->frustum.intersects(vmin,vmax))
    return;

  float mydist = (gWorld->camera - vcenter).length() - r;

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
    glDrawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, LineStrip);
  }
  else if( (px==15) && (py==0) )
  {
    glColor4f(0.0,1.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, LineStrip);
  }
  else if(px==15)
  {
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, LineStrip);
    glColor4f(0.0,1.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &LineStrip[8]);
  }
  else if(py==0)
  {
    glColor4f(0.0,1.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, LineStrip);
    glColor4f(1.0,0.0,0.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &LineStrip[8]);
  }

  if(Environment::getInstance()->view_holelines)
  {
    // Draw hole lines if view_subchunk_lines is true
    glColor4f(0.0,0.0,1.0f,0.5f);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, HoleStrip);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[9]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[18]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[27]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[36]);
    glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[45]);
  }

  glPopMatrix();
  glEnable(GL_LIGHTING);
  glColor4f(1,1,1,1);
}

void MapChunk::drawContour()
{
  if(!DrawMapContour)
    return;
  glColor4f(1,1,1,1);
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_ALPHA_TEST);
  if(Contour == 0)
    GenerateContourMap();
  glBindTexture(GL_TEXTURE_2D, Contour);

  glEnable(GL_TEXTURE_GEN_S);
  glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  glTexGenfv(GL_S,GL_OBJECT_PLANE,CoordGen);

  drawPass(-1);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
}

void MapChunk::draw()
{

  if (!gWorld->frustum.intersects( vmin, vmax ))
    return;

  float mydist = (gWorld->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  // setup vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, vertices);
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, normals);
  glNormalPointer(GL_FLOAT, 0, 0);
  // ASSUME: texture coordinates set up already


  // first pass: base texture
  if (textureSet->num() == 0U)
  {
    OpenGL::Texture::setActiveTexture( 0 );
    OpenGL::Texture::disableTexture();

    OpenGL::Texture::setActiveTexture( 1 );
    OpenGL::Texture::disableTexture();

    glColor3f(1.0f,1.0f,1.0f);
  }
  else
  {
    textureSet->bindTexture(0, 0);

    OpenGL::Texture::setActiveTexture( 1 );
    OpenGL::Texture::disableTexture();
  }

  glEnable(GL_LIGHTING);
  drawPass(-1);

  if (textureSet->num() > 1U) {
    //glDepthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    glDepthMask(GL_FALSE);
  }

  // additional passes: if required
  for( size_t i = 1; i < textureSet->num(); ++i )
  {
    // this time, use blending:
    textureSet->bindTexture(i, 0);
    textureSet->bindAlphamap(i-1, 1);

    drawPass(i);
  }

  if (textureSet->num() > 1U)
  {
    //glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
  }

  // shadow map
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  Vec3D shc = gWorld->skies->colorSet[WATER_COLOR_DARK] * 0.3f;
  glColor4f(shc.x,shc.y,shc.z,1);

  //glColor4f(1,1,1,1);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, shadow);
  glEnable(GL_TEXTURE_2D);

  drawPass(-1);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  drawContour();

  if(terrainMode==5)
  {
    // draw chunk white if impassible flag is set
    if(Flags & FLAG_IMPASS)
    {
      glColor4f(1,1,1,0.6f);
      drawPass(-1);
    }
  }

  if(terrainMode==4)
  {
    // draw chunks in color depending on AreaID and list color from environment
    if(Environment::getInstance()->areaIDColors.find(areaID) != Environment::getInstance()->areaIDColors.end() )
    {
      Vec3D colorValues = Environment::getInstance()->areaIDColors.find(areaID)->second;
      glColor4f(colorValues.x,colorValues.y,colorValues.z,0.7f);
      drawPass(-1);
    }
  }

  if(Environment::getInstance()->cursorType == 3)
  {
    if( gWorld->IsSelection( eEntry_MapChunk ) && gWorld->GetCurrentSelection()->data.mapchunk == this && terrainMode != 3 )
    {
      int poly = gWorld->GetCurrentSelectedTriangle();

      glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

      glPushMatrix();

      glDisable( GL_CULL_FACE );
      glDepthMask( false );
      glDisable( GL_DEPTH_TEST );
      glBegin( GL_TRIANGLES );
      glVertex3fv( mVertices[gWorld->mapstrip2[poly + 0]] );
      glVertex3fv( mVertices[gWorld->mapstrip2[poly + 1]] );
      glVertex3fv( mVertices[gWorld->mapstrip2[poly + 2]] );
      glEnd();
      glEnable( GL_CULL_FACE );
      glEnable( GL_DEPTH_TEST );
      glDepthMask( true );

      glPopMatrix();
    }
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

void MapChunk::drawSelect()
{
  if( !gWorld->frustum.intersects( vmin, vmax ) )
    return;

  float mydist = (gWorld->camera - vcenter).length() - r;
  if (mydist > (mapdrawdistance * mapdrawdistance)) return;
  if (mydist > gWorld->culldistance)
    return;

  if( nameID == -1 )
    nameID = SelectionNames.add( this );

  //! \todo Use backface culling again? Maybe this adds problems. Idk.
  //glDisable( GL_CULL_FACE );
  glPushName( nameID );

  for( int i = 0; i < stripsize2 - 2; ++i )
  {
    glPushName( i );
    glBegin( GL_TRIANGLES );
    glVertex3fv( mVertices[gWorld->mapstrip2[i]] );
    glVertex3fv( mVertices[gWorld->mapstrip2[i + 1]] );
    glVertex3fv( mVertices[gWorld->mapstrip2[i + 2]] );
    glEnd();
    glPopName();
  }

  glPopName();
  //glEnable( GL_CULL_FACE );
}

void MapChunk::getSelectionCoord( float *x, float *z )
{
  int Poly = gWorld->GetCurrentSelectedTriangle();
  if( Poly + 2 > stripsize2 )
  {
    *x = -1000000.0f;
    *z = -1000000.0f;
    return;
  }
  *x = ( mVertices[gWorld->mapstrip2[Poly + 0]].x + mVertices[gWorld->mapstrip2[Poly + 1]].x + mVertices[gWorld->mapstrip2[Poly + 2]].x ) / 3;
  *z = ( mVertices[gWorld->mapstrip2[Poly + 0]].z + mVertices[gWorld->mapstrip2[Poly + 1]].z + mVertices[gWorld->mapstrip2[Poly + 2]].z ) / 3;
}

float MapChunk::getSelectionHeight()
{
  int Poly = gWorld->GetCurrentSelectedTriangle();
  if( Poly + 2 < stripsize2 )
    return ( mVertices[gWorld->mapstrip2[Poly + 0]].y + mVertices[gWorld->mapstrip2[Poly + 1]].y + mVertices[gWorld->mapstrip2[Poly + 2]].y ) / 3;
  LogError << "Getting selection height fucked up because the selection was bad. " << Poly << "%i with striplen of " << stripsize2 << "." << std::endl;
  return 0.0f;
}

Vec3D MapChunk::GetSelectionPosition()
{
  int Poly = gWorld->GetCurrentSelectedTriangle();
  if( Poly + 2 > stripsize2 )
  {
    LogError << "Getting selection position fucked up because the selection was bad. " << Poly << "%i with striplen of " << stripsize2 << "." << std::endl;
    return Vec3D( -1000000.0f, -1000000.0f, -1000000.0f );
  }

  Vec3D lPosition;
  lPosition  = Vec3D( mVertices[gWorld->mapstrip2[Poly + 0]] );
  lPosition += Vec3D( mVertices[gWorld->mapstrip2[Poly + 1]] );
  lPosition += Vec3D( mVertices[gWorld->mapstrip2[Poly + 2]] );
  lPosition *= 0.3333333f;

  return lPosition;
}

void MapChunk::recalcNorms()
{

  Vec3D P1,P2,P3,P4;
  Vec3D Norm,N1,N2,N3,N4,D;


  if(Changed==false)
    return;
  Changed=false;

  for(int i=0;i<mapbufsize;++i)
  {
    if(!gWorld->GetVertex( mVertices[i].x - UNITSIZE*0.5f, mVertices[i].z - UNITSIZE*0.5f, &P1 ))
    {
      P1.x = mVertices[i].x - UNITSIZE*0.5f;
      P1.y = mVertices[i].y;
      P1.z = mVertices[i].z - UNITSIZE*0.5f;
    }

    if(!gWorld->GetVertex( mVertices[i].x + UNITSIZE*0.5f, mVertices[i].z - UNITSIZE*0.5f, &P2 ))
    {
      P2.x = mVertices[i].x + UNITSIZE*0.5f;
      P2.y = mVertices[i].y;
      P2.z = mVertices[i].z - UNITSIZE*0.5f;
    }

    if(!gWorld->GetVertex( mVertices[i].x + UNITSIZE*0.5f, mVertices[i].z + UNITSIZE*0.5f, &P3 ))
    {
      P3.x = mVertices[i].x + UNITSIZE*0.5f;
      P3.y = mVertices[i].y;
      P3.z = mVertices[i].z + UNITSIZE*0.5f;
    }

    if(!gWorld->GetVertex( mVertices[i].x - UNITSIZE*0.5f, mVertices[i].z + UNITSIZE*0.5f, &P4 ))
    {
      P4.x = mVertices[i].x - UNITSIZE*0.5f;
      P4.y = mVertices[i].y;
      P4.z = mVertices[i].z + UNITSIZE*0.5f;
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

  float ShadowAmount;
  for (int j=0; j<mapbufsize;++j)
  {
    //tm[j].z=tv[j].y;
    ShadowAmount=1.0f-(-mNormals[j].x+mNormals[j].y-mNormals[j].z);
    if(ShadowAmount<0)
      ShadowAmount=0;
    if(ShadowAmount>1.0)
      ShadowAmount=1.0f;
    ShadowAmount*=0.5f;
    //ShadowAmount=0.2;
    mFakeShadows[j].x=0;
    mFakeShadows[j].y=0;
    mFakeShadows[j].z=0;
    mFakeShadows[j].w=ShadowAmount;
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
  vmin.y = 9999999.0f;
  vmax.y = -9999999.0f;
  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x - x;
    zdiff = mVertices[i].z - z;
    if(BrushType == 5){
      if((abs(xdiff) < abs(radius/2)) && (abs(zdiff) < abs(radius/2))){
        mVertices[i].y += change;
        Changed=true;
      }
    }
    else
    {
      dist = sqrt(xdiff*xdiff + zdiff*zdiff);
      if(dist < radius)
      {

        if(BrushType==0)//Flat
          mVertices[i].y += change;

        else if(BrushType==1)//Linear
          mVertices[i].y += change*(1.0f - dist/radius);

        else if(BrushType==2)//Smooth
          mVertices[i].y += change/(1.0f + dist/radius);

        else if (BrushType == 3) //x^2
          mVertices[i].y += change*( (dist/radius)*(dist/radius) + dist/radius + 1.0f);

        else if (BrushType == 4) //cos
          mVertices[i].y += change*cos(dist/radius);
        Changed=true;
      }
    }

    vmin.y = std::min(vmin.y, mVertices[i].y);
    vmax.y = std::max(vmax.y, mVertices[i].y);
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
  float speed = 1.00f;
  float dist,xdiff,zdiff,nremain;
  Changed=false;

  xdiff= xbase - x + CHUNKSIZE/2;
  zdiff= zbase - z + CHUNKSIZE/2;
  dist= sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + MAPCHUNK_DIAMETER))
    return Changed;

  vmin.y = 9999999.0f;
  vmax.y = -9999999.0f;

  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x - x;
    zdiff = mVertices[i].z - z;

    dist=sqrt(xdiff*xdiff + zdiff*zdiff);

    if(dist < radius)
    {
      if(BrushType==0)//Flat
      {
        mVertices[i].y = remain*mVertices[i].y + (1 - remain)*h;
      }
      else if(BrushType==1)//Linear
      {
        nremain = 1 - (1 - remain) * (1 - dist/radius);
        mVertices[i].y = nremain*mVertices[i].y + (1-nremain)*h;
      }
      else if(BrushType==2)//Smooth
      {
        nremain = 1.0f - pow(1.0f - remain, (1.0f + dist/radius));
        mVertices[i].y = nremain*mVertices[i].y + ((1 - nremain)*h);
      }

      Changed=true;
    }

    vmin.y = std::min(vmin.y, mVertices[i].y);
    vmax.y = std::max(vmax.y, mVertices[i].y);
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

  vmin.y = 9999999.0f;
  vmax.y = -9999999.0f;

  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff= mVertices[i].x - x;
    zdiff= mVertices[i].z - z;

    dist= sqrt(xdiff*xdiff + zdiff*zdiff);

    if(dist < radius)
    {
      float TotalHeight;
      float TotalWeight;
      float tx,tz, h;
      Vec3D TempVec;
      int Rad=(radius/UNITSIZE);

      TotalHeight=0;
      TotalWeight=0;
      for(int j= -Rad*2; j <= Rad*2; ++j)
      {
        tz= z + j * UNITSIZE/2;
        for(int k=-Rad; k <= Rad; ++k)
        {
          tx= x + k*UNITSIZE + (j%2) * UNITSIZE/2.0f;
          xdiff= tx - mVertices[i].x;
          zdiff= tz - mVertices[i].z;
          dist2= sqrt(xdiff*xdiff + zdiff*zdiff);
          if(dist2 > radius)
            continue;
          gWorld->GetVertex(tx,tz,&TempVec);
          TotalHeight += (1.0f - dist2/radius) * TempVec.y;
          TotalWeight += (1.0f - dist2/radius);
        }
      }

      h=TotalHeight/TotalWeight;

      if(BrushType==0)//Flat
      {
        mVertices[i].y= remain * mVertices[i].y + (1 - remain) * h;
      }
      else if(BrushType==1)//Linear
      {
        nremain= 1 - (1 - remain) * (1 - dist/radius);
        mVertices[i].y= nremain * mVertices[i].y + ( 1 - nremain) * h;
      }
      else if(BrushType==2)//Smooth
      {
        nremain= 1.0f - pow( 1.0f - remain , (1.0f + dist/radius) );
        mVertices[i].y= nremain*mVertices[i].y + (1-nremain)*h;
      }

      Changed=true;
    }

    vmin.y = std::min(vmin.y, mVertices[i].y);
    vmax.y = std::max(vmax.y, mVertices[i].y);
  }
  if(Changed)
  {
    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}


void MapChunk::eraseTextures()
{
  textureSet->eraseTextures();
}

int MapChunk::addTexture( OpenGL::Texture* texture )
{
  return textureSet->addTexture(texture);
}

void MapChunk::switchTexture( OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture )
{
  textureSet->switchTexture(oldTexture, newTexture);
}

bool MapChunk::paintTexture( float x, float z, Brush* brush, float strength, float pressure, OpenGL::Texture* texture )
{
  return textureSet->paintTexture(xbase, zbase, x, z, brush, strength, pressure, texture);
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

void MapChunk::addHoleBig( int i, int j )
{
  for(int x=-3;x<4;x++)
  {
    for(int y=-3;y<4;y++)
    {
      addHole( i+x, j+y );
    }
  }
}

void MapChunk::removeHole( int i, int j )
{
  holes = holes & ~( ( 1 << ((j*4)+i)) );
  initStrip();
}

void MapChunk::removeHoleBig( int i,int j )
{
  for(int x=-3;x<4;x++)
  {
    for(int y=-3;y<4;y++)
    {
      removeHole( i+x, j+y );
    }
  }
}

void MapChunk::setAreaID( int ID )
{
  areaID = ID;
}

int MapChunk::getAreaID()
{
  return areaID;
}


void MapChunk::setFlag( bool changeto )
{
  if(changeto)
    this->Flags = this->Flags | (Environment::getInstance()->flagPaintMode);
  else
    this->Flags = this->Flags & ~(Environment::getInstance()->flagPaintMode);
}

void MapChunk::save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::map<int, WMOInstance> &lObjectInstances, std::map<int, ModelInstance> &lModelInstances)
{
  int lID;
  int lMCNK_Size = 0x80;
  int lMCNK_Position = lCurrentPosition;
  lADTFile.Extend( 8 + 0x80 );  // This is only the size of the header. More chunks will increase the size.
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCNK', lMCNK_Size );
  lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 )->mEntries[py*16+px].offset = lCurrentPosition; // check this

  // MCNK data
  lADTFile.Insert( lCurrentPosition + 8, 0x80, reinterpret_cast<char*>( &( header ) ) );
  MapChunkHeader * lMCNK_header = lADTFile.GetPointer<MapChunkHeader>( lCurrentPosition + 8 );

  lMCNK_header->flags = Flags;
  lMCNK_header->holes = holes;
  lMCNK_header->areaid = areaID;

  lMCNK_header->nLayers = -1;
  lMCNK_header->nDoodadRefs = -1;
  lMCNK_header->ofsHeight = -1;
  lMCNK_header->ofsNormal = -1;
  lMCNK_header->ofsLayer = -1;
  lMCNK_header->ofsRefs = -1;
  lMCNK_header->ofsAlpha = -1;
  lMCNK_header->sizeAlpha = -1;
  lMCNK_header->ofsShadow = -1;
  lMCNK_header->sizeShadow = -1;
  lMCNK_header->nMapObjRefs = -1;

  //! \todo  Implement sound emitter support. Or not.
  lMCNK_header->ofsSndEmitters = 0;
  lMCNK_header->nSndEmitters = 0;

  lMCNK_header->ofsLiquid = 0;
  //! \todo Is this still 8 if no chunk is present? Or did they correct that?
  lMCNK_header->sizeLiquid = 8;

  //! \todo  MCCV sub-chunk
  lMCNK_header->ofsMCCV = 0;

  if( lMCNK_header->flags & 0x40 )
    LogError << "Problem with saving: This ADT is said to have vertex shading but we don't write them yet. This might get you really fucked up results." << std::endl;
  lMCNK_header->flags = lMCNK_header->flags & ( ~0x40 );

  //really low tex map

  memset (lMCNK_header->low_quality_texture_map, 0, 0x10);

  for (size_t layer (0); layer < std::max((int)(textureSet->num() - 1), 0); ++layer)
  {
    for (size_t y (0); y < 8; ++y)
    {
      for (size_t x (0); x < 8; ++x)
      {
        size_t sum (0);

        for (size_t j (0); j < 8; ++j)
        {
          for (size_t i (0); i < 8; ++i)
          {
            sum += textureSet->getAlpha(layer, (y * 8 + j) * 64 + (x * 8 + i));

          }
        }

        static const size_t minimum_value_to_overwrite (120);

        if (sum > minimum_value_to_overwrite * 8 * 8)
        {
          const size_t array_index ((y * 8 + x) / 4);
          const size_t bit_index (((y * 8 + x) % 4) * 2);

          lMCNK_header->low_quality_texture_map[array_index] |= ((layer & 3) << bit_index);
        }
      }
    }
  }


  lCurrentPosition += 8 + 0x80;

  // MCVT
  //        {
  int lMCVT_Size = ( 9 * 9 + 8 * 8 ) * 4;

  lADTFile.Extend( 8 + lMCVT_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCVT', lMCVT_Size );

  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsHeight = lCurrentPosition - lMCNK_Position;

  float * lHeightmap = lADTFile.GetPointer<float>( lCurrentPosition + 8 );

  float lMedian = 0.0f;
  for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
    lMedian = lMedian + mVertices[i].y;

  lMedian = lMedian / ( 9 * 9 + 8 * 8 );
  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ypos = lMedian;

  for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
    lHeightmap[i] = mVertices[i].y - lMedian;

  lCurrentPosition += 8 + lMCVT_Size;
  lMCNK_Size += 8 + lMCVT_Size;
  //        }

  // MCNR
  //        {
  int lMCNR_Size = ( 9 * 9 + 8 * 8 ) * 3;

  lADTFile.Extend( 8 + lMCNR_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCNR', lMCNR_Size );

  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsNormal = lCurrentPosition - lMCNK_Position;

  char * lNormals = lADTFile.GetPointer<char>( lCurrentPosition + 8 );

  // recalculate the normals
  recalcNorms();
  for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
  {
    lNormals[i*3+0] = misc::roundc( -mNormals[i].z * 127 );
    lNormals[i*3+1] = misc::roundc( -mNormals[i].x * 127 );
    lNormals[i*3+2] = misc::roundc(  mNormals[i].y * 127 );
  }

  lCurrentPosition += 8 + lMCNR_Size;
  lMCNK_Size += 8 + lMCNR_Size;
  //        }

  // Unknown MCNR bytes
  // These are not in as we have data or something but just to make the files more blizzlike.
  //        {
  lADTFile.Extend( 13 );
  lCurrentPosition += 13;
  lMCNK_Size += 13;
  //        }

  // MCLY
  //        {
  size_t lMCLY_Size = textureSet->num() * 0x10;

  lADTFile.Extend( 8 + lMCLY_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size );

  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsLayer = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nLayers = textureSet->num();

  // MCLY data
  for( size_t j = 0; j < textureSet->num(); ++j )
  {
    ENTRY_MCLY * lLayer = lADTFile.GetPointer<ENTRY_MCLY>( lCurrentPosition + 8 + 0x10 * j );

    lLayer->textureID = lTextures.find( textureSet->filename(j) )->second;

    lLayer->flags = textureSet->flag(j);

    // if not first, have alpha layer, if first, have not. never have compression.
    lLayer->flags = ( j > 0 ? lLayer->flags | FLAG_USE_ALPHA : lLayer->flags & ( ~FLAG_USE_ALPHA ) ) & ( ~FLAG_ALPHA_COMPRESSED );

    lLayer->ofsAlpha = ( j == 0 ? 0 : ( mBigAlpha ? 64 * 64 * ( j - 1 ) : 32 * 64 * ( j - 1 ) ) );
    lLayer->effectID = textureSet->effect(j);
  }

  lCurrentPosition += 8 + lMCLY_Size;
  lMCNK_Size += 8 + lMCLY_Size;
  //        }

  // MCRF
  //        {
  std::list<int> lDoodadIDs;
  std::list<int> lObjectIDs;

  Vec3D lChunkExtents[2];
  lChunkExtents[0] = Vec3D( xbase, 0.0f, zbase );
  lChunkExtents[1] = Vec3D( xbase + CHUNKSIZE, 0.0f, zbase + CHUNKSIZE );

  // search all wmos that are inside this chunk
  lID = 0;
  for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
  {
    //! \todo  This requires the extents already being calculated. See above.
    if(it->second.isInsideTile(lChunkExtents))
      lObjectIDs.push_back( lID );

    lID++;
  }

  // search all models that are inside this chunk
  lID = 0;
  for( std::map<int, ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    // get radius and position of the m2
    float radius = it->second.model->header.BoundingBoxRadius;
    Vec3D& pos = it->second.pos;

    // Calculate the chunk zenter
    Vec3D chunkMid(xbase + CHUNKSIZE / 2, 0,
                   zbase + CHUNKSIZE / 2);

    // find out if the model is inside the reach of the chunk.
    float dx = chunkMid.x - pos.x;
    float dz = chunkMid.z - pos.z;
    float dist = sqrtf(dx * dx + dz * dz);
    static float sqrt2 = sqrtf(2.0f);

    if(dist - radius <= ((sqrt2 / 2.0f) * CHUNKSIZE))
    {
      lDoodadIDs.push_back(lID);
    }

    lID++;
  }

  int lMCRF_Size = 4 * ( lDoodadIDs.size() + lObjectIDs.size() );
  lADTFile.Extend( 8 + lMCRF_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCRF', lMCRF_Size );

  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsRefs = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nDoodadRefs = lDoodadIDs.size();
  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nMapObjRefs = lObjectIDs.size();

  // MCRF data
  int * lReferences = lADTFile.GetPointer<int>( lCurrentPosition + 8 );

  lID = 0;
  for( std::list<int>::iterator it = lDoodadIDs.begin(); it != lDoodadIDs.end(); ++it )
  {
    lReferences[lID] = *it;
    lID++;
  }

  for( std::list<int>::iterator it = lObjectIDs.begin(); it != lObjectIDs.end(); ++it )
  {
    lReferences[lID] = *it;
    lID++;
  }

  lCurrentPosition += 8 + lMCRF_Size;
  lMCNK_Size += 8 + lMCRF_Size;
  //        }

  // MCSH
  //        {
  //! \todo  Somehow determine if we need to write this or not?
  //! \todo  This sometime gets all shadows black.
  if( Flags & 1 )
  {
    int lMCSH_Size = 0x200;
    lADTFile.Extend( 8 + lMCSH_Size );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size );

    lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsShadow = lCurrentPosition - lMCNK_Position;
    lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeShadow = 0x200;

    char * lLayer = lADTFile.GetPointer<char>( lCurrentPosition + 8 );

    memcpy( lLayer, mShadowMap, 0x200 );

    lCurrentPosition += 8 + lMCSH_Size;
    lMCNK_Size += 8 + lMCSH_Size;
  }
  else
  {
    lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsShadow = 0;
    lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeShadow = 0;
  }
  //        }

  // MCAL
  //        {
  int lDimensions = 64 * ( mBigAlpha ? 64 : 32 );

  size_t lMaps = textureSet->num() ? textureSet->num() - 1U : 0U;

  int lMCAL_Size = lDimensions * lMaps;

  lADTFile.Extend( 8 + lMCAL_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCAL', lMCAL_Size );

  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsAlpha = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeAlpha = 8 + lMCAL_Size;

  char * lAlphaMaps = lADTFile.GetPointer<char>( lCurrentPosition + 8 );

  for( size_t j = 0; j < lMaps; j++ )
  {
    //First thing we have to do is downsample the alpha maps before we can write them
    if( mBigAlpha )
      for( int k = 0; k < lDimensions; k++ )
        lAlphaMaps[lDimensions * j + k] = textureSet->getAlpha(j, k);
    else
    {
      unsigned char upperNibble, lowerNibble;
      for( int k = 0; k < lDimensions; k++ )
      {
        lowerNibble = static_cast<unsigned char>(std::max(std::min( ( static_cast<float>(textureSet->getAlpha(j, k * 2 + 0)) ) * 0.05882f + 0.5f , 15.0f),0.0f));
        upperNibble = static_cast<unsigned char>(std::max(std::min( ( static_cast<float>(textureSet->getAlpha(j, k * 2 + 1)) ) * 0.05882f + 0.5f , 15.0f),0.0f));
        lAlphaMaps[lDimensions * j + k] = ( upperNibble << 4 ) + lowerNibble;
      }
    }
  }

  lCurrentPosition += 8 + lMCAL_Size;
  lMCNK_Size += 8 + lMCAL_Size;
  //        }

  //! Don't write anything MCLQ related anymore...

  // MCSE
  //        {
  int lMCSE_Size = 0;
  lADTFile.Extend( 8 + lMCSE_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCSE', lMCSE_Size );

  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsSndEmitters = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nSndEmitters = lMCSE_Size / 0x1C;

  // if ( data ) do write

  /*
    if(sound_Exist){
    memcpy(&Temp,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters+4,sizeof(int));
    memcpy(Buffer+Change+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters+lChange,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters,Temp+8);
    ChunkHeader[i].ofsSndEmitters+=lChange;
    }
    */

  lCurrentPosition += 8 + lMCSE_Size;
  lMCNK_Size += 8 + lMCSE_Size;
  //        }



  lADTFile.GetPointer<sChunkHeader>( lMCNK_Position )->mSize = lMCNK_Size;
  lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 )->mEntries[py*16+px].size = lMCNK_Size;
}


//! ------ unused functions -----

/*
void MapChunk::drawNoDetail()
{
  glActiveTexture( GL_TEXTURE1 );
  glDisable( GL_TEXTURE_2D );
  glActiveTexture(GL_TEXTURE0 );
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_LIGHTING );

  //glColor3fv(gWorld->skies->colorSet[FOG_COLOR]);
  //glColor3f(1,0,0);
  //glDisable(GL_FOG);

  // low detail version
  glBindBuffer( GL_ARRAY_BUFFER, vertices );
  glVertexPointer( 3, GL_FLOAT, 0, 0 );
  glDisableClientState( GL_NORMAL_ARRAY );
  glDrawElements( GL_TRIANGLE_STRIP, stripsize, GL_UNSIGNED_SHORT, gWorld->mapstrip );
  glEnableClientState( GL_NORMAL_ARRAY );

  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
  //glEnable(GL_FOG);

  glEnable( GL_LIGHTING );
  glActiveTexture( GL_TEXTURE1 );
  glEnable( GL_TEXTURE_2D );
  glActiveTexture( GL_TEXTURE0 );
  glEnable( GL_TEXTURE_2D );
}
*/

/*
void MapChunk::drawColor()
{

  if (!gWorld->frustum.intersects(vmin,vmax))
    return;

  float mydist = (gWorld->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  if (mydist > gWorld->culldistance) {
    if (gWorld->drawfog) this->drawNoDetail();
    return;
  }

  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);

  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  //glDisable(GL_LIGHTING);

  Vec3D Color;
  glBegin(GL_TRIANGLE_STRIP);
  for(int i=0; i < striplen; ++i)
  {
    HeightColor( mVertices[strip[i]].y, &Color);
    glColor3fv(&Color.x);
    glNormal3fv(&mNormals[strip[i]].x);
    glVertex3fv(&mVertices[strip[i]].x);
  }
  glEnd();
  //glEnable(GL_LIGHTING);
}
*/

/*
void MapChunk::loadTextures()
{
  //! \todo Use this kind of preloading again?
  return;
    for(int i=0; i < nTextures; ++i)
    _textures[i] = TextureManager::get(mt->mTextureFilenames[tex[i]]);
}
*/



/*void HeightColor(float height, Vec3D *Color)
{
  White  1.00  1.00  1.00
  Brown  0.75  0.50  0.00
  Green  0.00  1.00  0.00
  Yellow  1.00  1.00  0.00
  Lt Blue  0.00  1.00  1.00
  Blue  0.00  0.00  1.00
  Black  0.00  0.00  0.00

  float Amount;

  if(height>HEIGHT_TOP)
  {
    Color->x=1.0;
    Color->y=1.0;
    Color->z=1.0;
  }
  else if(height>HEIGHT_MID)
  {
    Amount=(height-HEIGHT_MID)/(HEIGHT_TOP-HEIGHT_MID);
    Color->x=.75f+Amount*0.25f;
    Color->y=0.5f+0.5f*Amount;
    Color->z=Amount;
  }
  else if(height>HEIGHT_LOW)
  {
    Amount=(height-HEIGHT_LOW)/(HEIGHT_MID-HEIGHT_LOW);
    Color->x=Amount*0.75f;
    Color->y=1.00f-0.5f*Amount;
    Color->z=0.0f;
  }
  else if(height>HEIGHT_ZERO)
  {
    Amount=(height-HEIGHT_ZERO)/(HEIGHT_LOW-HEIGHT_ZERO);

    Color->x=1.0f-Amount;
    Color->y=1.0f;
    Color->z=0.0f;
  }
  else if(height>HEIGHT_SHALLOW)
  {
    Amount=(height-HEIGHT_SHALLOW)/(HEIGHT_ZERO-HEIGHT_SHALLOW);
    Color->x=0.0f;
    Color->y=Amount;
    Color->z=1.0f;
  }
  else if(height>HEIGHT_DEEP)
  {
    Amount=(height-HEIGHT_DEEP)/(HEIGHT_SHALLOW-HEIGHT_DEEP);
    Color->x=0.0f;
    Color->y=0.0f;
    Color->z=Amount;
  }
  else
    (*Color)*=0.0f;

}*/
