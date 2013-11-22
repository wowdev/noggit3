#include "World.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <time.h>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#include "DBC.h"
#include "Environment.h"
#include "Log.h"
#include "MapChunk.h"
#include "MapTile.h"
#include "Misc.h"
#include "ModelManager.h" // ModelManager
#include "Project.h"
#include "Settings.h"
#include "TextureManager.h"
#include "UITexturingGUI.h"
#include "Video.h"
#include "WMOInstance.h" // WMOInstance
#include "MapTile.h"
#include "Brush.h" // brush
#include "ConfigFile.h"
#include "MapIndex.h"
#include "TileWater.h"// tile water

World *gWorld = NULL;

GLuint selectionBuffer[8192];


void renderSphere(float x1, float y1, float z1, float x2, float y2, float z2, float radius, int subdivisions, GLUquadricObj *quadric)
{
  float vx = x2-x1;
  float vy = y2-y1;
  float vz = z2-z1;

  //handle the degenerate case of z1 == z2 with an approximation
  if( vz == 0.0f )
    vz = .0001f;

  float v = sqrt( vx*vx + vy*vy + vz*vz );
  float ax = 57.2957795f*acos( vz/v );
  if ( vz < 0.0f )
    ax = -ax;
  float rx = -vy*vz;
  float ry = vx*vz;
  glPushMatrix();

  //draw the quadric
  glTranslatef( x1,y1,z1 );
  glRotatef(ax, rx, ry, 0.0);

  gluQuadricOrientation(quadric,GLU_OUTSIDE);
  gluSphere(quadric, radius, subdivisions , subdivisions );

  glPopMatrix();
}

void renderSphere_convenient(float x, float y, float z, float radius, int subdivisions)
{
  if(Environment::getInstance()->screenX>0 && Environment::getInstance()->screenY>0)
  {
    //the same quadric can be re-used for drawing many objects
    glDisable(GL_LIGHTING);
    glColor4f(Environment::getInstance()->cursorColorR, Environment::getInstance()->cursorColorG, Environment::getInstance()->cursorColorB, Environment::getInstance()->cursorColorA );
    GLUquadricObj *quadric=gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    renderSphere(x,y,z,x,y,z,0.3f,15,quadric);
    renderSphere(x,y,z,x,y,z,radius,subdivisions,quadric);
    gluDeleteQuadric(quadric);
    glEnable(GL_LIGHTING);
  }
}

void renderDisk(float x1, float y1, float z1, float x2, float y2, float z2, float radius, int subdivisions, GLUquadricObj *quadric)
{
  float vx = x2 - x1;
  float vy = y2 - y1;
  float vz = z2 - z1;

  //handle the degenerate case of z1 == z2 with an approximation
  if( vz == 0.0f )
    vz = .0001f;

  float v = sqrt( vx*vx + vy*vy + vz*vz );
  float ax = 57.2957795f*acos( vz/v );
  if(vz < 0.0f)
    ax = -ax;

  float rx = -vy * vz;
  float ry = vx * vz;
  
  glLineWidth (2.0);
  //   glEnable (GL_LINE_STIPPLE);
  //   glLineStipple(2, 0x00FF);
  glPushMatrix();
  glDisable(GL_DEPTH_TEST);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  //draw the quadric
  glTranslatef(x1, y1, z1);
  glRotatef(ax, rx, ry, 0.0f);
  glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
  glColor4f(Environment::getInstance()->cursorColorR, Environment::getInstance()->cursorColorG, Environment::getInstance()->cursorColorB, Environment::getInstance()->cursorColorA);

  gluQuadricOrientation(quadric, GLU_OUTSIDE);
  gluDisk(quadric, radius, radius + 0.01f, subdivisions, 1);

  //glColor4f(0.0f, 0.8f, 0.1f, 0.9f);
  //gluDisk(quadric, (radius * 1.5) - 2, (radius * 1.5) + 2, 0, 1);
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
}

void renderDisk_convenient(float x, float y, float z, float radius)
{
  int subdivisions =(int)radius * 3.5;
  if( subdivisions < 35 ) subdivisions=35;
  glDisable(GL_LIGHTING);
  GLUquadricObj *quadric = gluNewQuadric();
  gluQuadricDrawStyle(quadric, GLU_LINE);
  gluQuadricNormals(quadric, GLU_SMOOTH);
  renderDisk(x, y, z, x, y, z, radius, subdivisions, quadric);
  renderSphere(x,y,z,x,y,z,0.05,15,quadric);
  gluDeleteQuadric(quadric);
  glEnable(GL_LIGHTING);
}

bool World::IsEditableWorld( int pMapId )
{
  std::string lMapName;
  try
  {
    DBCFile::Record map = gMapDB.getByID( pMapId );
    lMapName = map.getString( MapDB::InternalName );
  }
  catch( ... )
  {
    LogError << "Did not find map with id " << pMapId << ". This is NOT editable.." << std::endl;
    return false;
  }

  std::stringstream ssfilename;
  ssfilename << "World\\Maps\\" << lMapName << "\\" << lMapName << ".wdt";

  if( !MPQFile::exists( ssfilename.str() ) )
  {
    Log << "World " << pMapId << ": " << lMapName << " has no WDT file!" << std::endl;
    return false;
  }

  MPQFile mf( ssfilename.str() );

  //sometimes, wdts don't open, so ignore them...
  if(mf.isEof())
    return false;

  const char * lPointer = reinterpret_cast<const char*>( mf.getPointer() );

  // Not using the libWDT here doubles performance. You might want to look at your lib again and improve it.
  const int lFlags = *( reinterpret_cast<const int*>( lPointer + 8 + 4 + 8 ) );
  if( lFlags & 1 )
    return false;

  const int * lData = reinterpret_cast<const int*>( lPointer + 8 + 4 + 8 + 0x20 + 8 );
  for( int i = 0; i < 8192; i += 2 )
  {
    if( lData[i] & 1 )
      return true;
  }

  return false;
}

World::World( const std::string& name )
  : ex( -1 )
  , ez( -1 )
  , cx(-1)
  , cz(-1)
  , mCurrentSelection( NULL )
  , mCurrentSelectedTriangle( 0 )
  , SelectionMode( false )
  , mWmoFilename( "" )
  , mWmoEntry( ENTRY_MODF() )
  , detailtexcoords( 0 )
  , alphatexcoords( 0 )
  , mMapId( 0xFFFFFFFF )
  , ol( NULL )
  , l_const( 0.0f )
  , l_linear( 0.7f )
  , l_quadratic( 0.03f )
  , drawdoodads( true )
  , drawfog( false )
  , drawlines( false )
  , drawmodels( true )
  , drawterrain( true )
  , drawwater( true )
  , drawwmo( true )
  , drawwireframe( false )
  , lighting( true )
  , animtime( 0 )
  , time( 1450 )
  , basename( name )
  , fogdistance( 777.0f )
  , culldistance( fogdistance )
  , autoheight( false )
  , minX( 0.0f )
  , maxX( 0.0f )
  , minY( 0.0f )
  , maxY( 0.0f )
  , zoom( 0.25f )
  , skies( NULL )
  , loading( false )
  , hadSky( false )
  , outdoorLightStats( OutdoorLightStats() )
  , minimap( 0 )
  , mapstrip( NULL )
  , mapstrip2( NULL )
  , camera( Vec3D( 0.0f, 0.0f, 0.0f ) )
  , lookat( Vec3D( 0.0f, 0.0f, 0.0f ) )
  , frustum( Frustum() )
{
  for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i )
  {
    if( name == std::string( i->getString( MapDB::InternalName ) ) )
    {
      mMapId = i->getUInt( MapDB::MapID );
      break;
    }
  }
  if( mMapId == 0xFFFFFFFF )
    LogError << "MapId for \"" << name << "\" not found! What is wrong here?" << std::endl;

  LogDebug << "Loading world \"" << name << "\"." << std::endl;

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      lowrestiles[j][i] = NULL;
    }
  }

  mapIndex = new MapIndex(basename);

  if(!mapIndex->hasAGlobalWMO())
    initMinimap();

}

void World::initMinimap()
{
  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";

  MPQFile f(filename.str());
  if (f.isEof()) {
    LogError << "file \"World\\Maps\\" << basename << "\\" << basename << ".wdl\" does not exist." << std::endl;
    return;
  }

  int ofsbuf[64][64];
  memset(ofsbuf, 0, 64*64*4);

  int fourcc(0);
  size_t size(0);

  while (!f.isEof()) {
    f.read(&fourcc,4);
    f.read(&size, 4);

    size_t nextpos = f.getPos() + size;

    /*  if( fourcc == 'MVER' ) {
    }
    else if( fourcc == 'MWMO' ) {
      // Filenames for WMO that appear in the low resolution map. Zero terminated strings.
    }
    else if( fourcc == 'MWID' ) {
      // List of indexes into the MWMO chunk.
    }
    else if( fourcc == 'MODF' ) {
      // Placement information for the WMO. Appears to be the same 64 byte structure used in the WDT and ADT MODF chunks.
    }
    else*/ if( fourcc == 'MAOF' ) {
      f.read(ofsbuf,64*64*4);
    }
    else if( fourcc == 'MARE' ) {
      glGenTextures(1, &minimap);

      // zomg, data on the stack!!1
      //int texbuf[512][512];
      unsigned int *texbuf = new unsigned int[512*512];
      memset(texbuf,0,512*512*4);

      // as alpha is unused, maybe I should try 24bpp? :(
      int16_t tilebuf[17*17];

      for (int j=0; j<64; ++j) {
        for (int i=0; i<64; ++i) {
          if (ofsbuf[j][i]) {
            f.seek(ofsbuf[j][i]+8);
            // read height values ^_^

            /*
            short *sp = tilebuf;
            for (int z=0; z<33; z++) {
              f.read(sp, 2 * ( (z%2) ? 16 : 17 ));
              sp += 17;
            }*/
            /*
            fucking win. in the .adt files, height maps are stored in 9-8-9-8-... interleaved order.
            here, apparently, a 17x17 map is stored followed by a 16x16 map.
            yay for consistency.
            I'm only using the 17x17 map here.
            */
            f.read(tilebuf,17*17*2);

            // make minimap
            // for a 512x512 minimap texture, and 64x64 tiles, one tile is 8x8 pixels
            for (int z=0; z<8; z++) {
              for (int x=0; x<8; x++) {
                int16_t hval = tilebuf[(z*2)*17+x*2]; // for now

                // make rgb from height value
                unsigned char r,g,b;
                if (hval < 0) {
                  // water = blue
                  if (hval < -511) hval = -511;
                  hval /= -2;
                  r = g = 0;
                  b = 255 - hval;
                } else {
                  // above water = should apply a palette :(
                  /*
                  float fh = hval / 1600.0f;
                  if (fh > 1.0f) fh = 1.0f;
                  unsigned char c = (unsigned char) (fh * 255.0f);
                  r = g = b = c;
                  */

                  // green: 20,149,7    0-600
                  // brown: 137, 84, 21  600-1200
                  // gray: 96, 96, 96    1200-1600
                  // white: 255, 255, 255
                  unsigned char r1,r2,g1,g2,b1,b2;
                  float t;

                  if (hval < 600) {
                    r1 = 20;
                    r2 = 137;
                    g1 = 149;
                    g2 = 84;
                    b1 = 7;
                    b2 = 21;
                    t = hval / 600.0f;
                  }
                  else if (hval < 1200) {
                    r2 = 96;
                    r1 = 137;
                    g2 = 96;
                    g1 = 84;
                    b2 = 96;
                    b1 = 21;
                    t = (hval-600) / 600.0f;
                  }
                  else /*if (hval < 1600)*/ {
                    r1 = 96;
                    r2 = 255;
                    g1 = 96;
                    g2 = 255;
                    b1 = 96;
                    b2 = 255;
                    if (hval >= 1600) hval = 1599;
                    t = (hval-1200) / 600.0f;
                  }

                  //! \todo  add a regular palette here

                  r = (unsigned char)(r2*t + r1*(1.0f-t));
                  g = (unsigned char)(g2*t + g1*(1.0f-t));
                  b = (unsigned char)(b2*t + b1*(1.0f-t));
                }

                texbuf[(j*8+z)*512 + i*8+x] = (r) | (g<<8) | (b<<16) | (255 << 24);
              }
            }
          }
        }
      }

      glBindTexture(GL_TEXTURE_2D, minimap);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, texbuf);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      delete[] texbuf;
      f.close();
      return;
    }
    else if( fourcc == 'MAHO' ) {
      /*
After each MARE chunk there follows a MAHO (MapAreaHOles) chunk. It may be left out if the data is supposed to be 0 all the time.
Its an array of 16 shorts. Each short is a bitmask. If the bit is not set, there is a hole at this position.
*/
    }
    /*  else  {
      char fcc[5];
      f.seekRelative(-8);
      f.read(fcc,4);
      fcc[4] = 0;
      gLog("minimap %s [%d].\n", fcc, size);
    } */
    f.seek(nextpos);
  }

  f.close();
}


void World::initLowresTerrain()
{
  //! \todo god dammn! this is so fucking expensive, change it, use VBOs or sth

  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";

  int16_t tilebuf[17*17];
  int16_t tilebuf2[16*16];
  Vec3D lowres[17][17];
  Vec3D lowsub[16][16];
  int32_t ofsbuf[64][64];

  MPQFile f(filename.str());

  int32_t fourcc;
  size_t size;

  while (!f.isEof())
  {
    f.read(&fourcc,4);
    f.read(&size, 4);

    if (size == 0)
      continue;

    size_t nextpos = f.getPos() + size;

    if( fourcc == 'MAOF' )
    {
      f.read(ofsbuf,64*64*4);

      for (size_t j=0; j<64; ++j)
      {
        for (size_t i=0; i<64; ++i)
        {
          if (ofsbuf[j][i])
          {
            f.seek(ofsbuf[j][i]+8);
            f.read(tilebuf,17*17*2);
            f.read(tilebuf2,16*16*2);

            for (size_t y=0; y<17; y++)
            {
              for (size_t x=0; x<17; x++)
              {
                lowres[y][x] = Vec3D(TILESIZE*(i+x/16.0f), tilebuf[y*17+x], TILESIZE*(j+y/16.0f));
              }
            }
            for (size_t y=0; y<16; y++)
            {
              for (size_t x=0; x<16; x++)
              {
                lowsub[y][x] = Vec3D(TILESIZE*(i+(x+0.5f)/16.0f), tilebuf2[y*16+x], TILESIZE*(j+(y+0.5f)/16.0f));
              }
            }

            lowrestiles[j][i] = new OpenGL::CallList();
            lowrestiles[j][i]->startRecording();

            glBegin( GL_TRIANGLES );
            for( size_t y = 0; y < 16; y++ )
            {
              for( size_t x = 0; x < 16; x++ )
              {
                glVertex3fv( lowres[y][x] );
                glVertex3fv( lowsub[y][x] );
                glVertex3fv( lowres[y][x+1] );
                glVertex3fv( lowres[y][x+1] );
                glVertex3fv( lowsub[y][x] );
                glVertex3fv( lowres[y+1][x+1] );
                glVertex3fv( lowres[y+1][x+1] );
                glVertex3fv( lowsub[y][x]) ;
                glVertex3fv( lowres[y+1][x] );
                glVertex3fv( lowres[y+1][x] );
                glVertex3fv( lowsub[y][x] );
                glVertex3fv( lowres[y][x] );
              }
            }
            glEnd();

            lowrestiles[j][i]->endRecording();

            /* OLD:
             // draw tiles 16x16?
             glBegin(GL_TRIANGLE_STRIP);
             for (int y=0; y<16; y++) {
             // end jump
             if (y>0) glVertex3fv(lowres[y][0]);
             for (int x=0; x<17; x++) {
             glVertex3fv(lowres[y][x]);
             glVertex3fv(lowres[y+1][x]);
             }
             // start jump
             if (y<15) glVertex3fv(lowres[y+1][16]);
             }
             glEnd();
             */
            // draw tiles 17*17+16*16
          }
        }
      }
      f.close();
      return;
    }
    f.seek(nextpos);
  }

  LogError << "Error in reading low res terrain. MAOF not found." << std::endl;
  f.close();
}

void World::initGlobalVBOs( GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords )
{
  if( !*pDetailTexCoords && !*pAlphaTexCoords )
  {
    Vec2D temp[mapbufsize], *vt;
    float tx,ty;

    // init texture coordinates for detail map:
    vt = temp;
    const float detail_half = 0.5f * detail_size / 8.0f;
    for (int j=0; j<17; ++j) {
      for (int i=0; i<((j%2)?8:9); ++i) {
        tx = detail_size / 8.0f * i;
        ty = detail_size / 8.0f * j * 0.5f;
        if (j%2) {
          // offset by half
          tx += detail_half;
        }
        *vt++ = Vec2D(tx, ty);
      }
    }

    glGenBuffers(1, pDetailTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, *pDetailTexCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(temp), temp, GL_STATIC_DRAW);

    // init texture coordinates for alpha map:
    vt = temp;

    const float alpha_half = 0.5f * (62.0f/64.0f) / 8.0f;
    for (int j=0; j<17; ++j) {
      for (int i=0; i<((j%2)?8:9); ++i) {
        tx = (62.0f/64.0f) / 8.0f * i;
        ty = (62.0f/64.0f) / 8.0f * j * 0.5f;
        if (j%2) {
          // offset by half
          tx += alpha_half;
        }
        *vt++ = Vec2D(tx, ty);
      }
    }

    glGenBuffers(1, pAlphaTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, *pAlphaTexCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(temp), temp, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}


void World::initDisplay()
{
  // default strip indices
  /* unused
  StripType *defstrip = new StripType[stripsize2];
  for (int i=0; i<stripsize; ++i) defstrip[i] = i; // note: this is ugly and should be handled in stripify
  mapstrip = new StripType[stripsize];
  stripify<StripType>(defstrip, mapstrip);
  delete[] defstrip;
  */

  StripType *defstrip = new StripType[stripsize2];
  for (int i=0; i<stripsize2; ++i) defstrip[i] = i; // note: this is ugly and should be handled in stripify
  mapstrip2 = new StripType[stripsize2];
  stripify2<StripType>(defstrip, mapstrip2);
  delete[] defstrip;

  initGlobalVBOs( &detailtexcoords, &alphatexcoords );

  mapIndex->setAdt(false);

  if(mapIndex->hasAGlobalWMO())
  {
    WMOInstance inst( WMOManager::add( mWmoFilename ), &mWmoEntry );

    gWorld->mWMOInstances.insert( std::pair<int,WMOInstance>( mWmoEntry.uniqueID, inst ) );
    camera = inst.pos;
  }

  skies = new Skies( mMapId );

  ol = new OutdoorLighting("World\\dnc.db");

  initLowresTerrain();
}

World::~World()
{

  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( lowrestiles[j][i] )
      {
        delete lowrestiles[j][i];
        lowrestiles[j][i] = NULL;
      }
    }
  }

  if (minimap)
  {
    glDeleteTextures(1, &minimap);
    minimap = 0;
  }

  if (skies)
  {
    delete skies;
    skies = NULL;
  }
  if (ol)
  {
    delete ol;
    ol = NULL;
  }

  if (mapstrip)
  {
    delete[] mapstrip;
    mapstrip = NULL;
  }
  if (mapstrip2)
  {
    delete[] mapstrip2;
    mapstrip2 = NULL;
  }

  LogDebug << "Unloaded world \"" << basename << "\"." << std::endl;
}


/*
void lightingDefaults()
{
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);

  glEnable(GL_LIGHT0);
  // wtf
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  glDisable(GL_LIGHT3);
  glDisable(GL_LIGHT4);
  glDisable(GL_LIGHT5);
  glDisable(GL_LIGHT6);
  glDisable(GL_LIGHT7);
}


void myFakeLighting()
{
  GLfloat la = 0.5f;
  GLfloat ld = 1.0f;

  GLfloat LightAmbient[] = {la, la, la, 1.0f};
  GLfloat LightDiffuse[] = {ld, ld, ld, 1.0f};
  GLfloat LightPosition[] = {-10.0f, 20.0f, -10.0f, 0.0f};
  glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
}
*/

void World::outdoorLighting()
{
  Vec4D black(0,0,0,0);
  Vec4D ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  float di = outdoorLightStats.dayIntensity;
  //float ni = outdoorLightStats.nightIntensity;

  Vec3D dd = outdoorLightStats.dayDir;
  // HACK: let's just keep the light source in place for now
  //Vec4D pos(-1, 1, -1, 0);
  Vec4D pos(-dd.x, -dd.z, dd.y, 0.0f);
  Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1.0f);
  glLightfv(GL_LIGHT0, GL_AMBIENT, black);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  /*
  dd = outdoorLightStats.nightDir;
  pos(-dd.x, -dd.z, dd.y, 0.0f);
  col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1.0f);
  glLightfv(GL_LIGHT1, GL_AMBIENT, black);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT1, GL_POSITION, pos);*/
}

/*void World::outdoorLighting2()
{
  Vec4D black(0,0,0,0);
  Vec4D ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  float di = outdoorLightStats.dayIntensity, ni = outdoorLightStats.nightIntensity;
  di = 1;
  ni = 0;

  //Vec3D dd = outdoorLightStats.dayDir;
  // HACK: let's just keep the light source in place for now
  Vec4D pos(-1, -1, -1, 0);
  Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1);
  glLightfv(GL_LIGHT0, GL_AMBIENT, black);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  */
/*
  Vec3D dd = outdoorLightStats.nightDir;
  Vec4D pos(-dd.x, -dd.z, dd.y, 0);
  Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1);
  glLightfv(GL_LIGHT1, GL_AMBIENT, black);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT1, GL_POSITION, pos);
  */ /*
}*/


void World::outdoorLights(bool on)
{
  float di = outdoorLightStats.dayIntensity;
  float ni = outdoorLightStats.nightIntensity;

  if (on) {
    Vec4D ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    if (di>0) {
      glEnable(GL_LIGHT0);
    } else {
      glDisable(GL_LIGHT0);
    }
    if (ni>0) {
      glEnable(GL_LIGHT1);
    } else {
      glDisable(GL_LIGHT1);
    }
  } else {
    Vec4D ambient(0, 0, 0, 1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
  }
}

void World::setupFog()
{
  if (drawfog) {

    //float fogdist = 357.0f; // minimum draw distance in wow
    //float fogdist = 777.0f; // maximum draw distance in wow

    float fogdist = fogdistance;
    float fogstart = 0.5f;

    culldistance = fogdist;

    //FOG_COLOR
    Vec4D fogcolor(skies->colorSet[FOG_COLOR], 1);
    glFogfv(GL_FOG_COLOR, fogcolor);
    //! \todo  retreive fogstart and fogend from lights.lit somehow
    glFogf(GL_FOG_END, fogdist);
    glFogf(GL_FOG_START, fogdist * fogstart);

    glEnable(GL_FOG);
  } else {
    glDisable(GL_FOG);
    culldistance = mapdrawdistance;
  }
}

extern float groundBrushRadius;
extern float blurBrushRadius;
extern int terrainMode;
extern Brush textureBrush;


void World::draw()
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  gluLookAt(camera.x,camera.y,camera.z, lookat.x,lookat.y,lookat.z, 0, 1, 0);

  frustum.retrieve();

  ///glDisable(GL_LIGHTING);
  ///glColor4f(1,1,1,1);World::draw()

  hadSky = false;
  if(drawwmo || mapIndex->hasAGlobalWMO())
    for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); !hadSky && it != mWMOInstances.end(); ++it )
      it->second.wmo->drawSkybox( this->camera, it->second.extents[0], it->second.extents[1] );

  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);

  int daytime = static_cast<int>(time) % 2880;
  outdoorLightStats = ol->getLightStats(daytime);
  skies->initSky(camera, daytime);

  if (!hadSky)
    hadSky = skies->drawSky(camera);

  // clearing the depth buffer only - color buffer is/has been overwritten anyway
  // unless there is no sky OR skybox
  GLbitfield clearmask = GL_DEPTH_BUFFER_BIT;
  if (!hadSky)   clearmask |= GL_COLOR_BUFFER_BIT;
  glClear(clearmask);

  glDisable(GL_TEXTURE_2D);

  outdoorLighting();
  outdoorLights(true);

  glFogi(GL_FOG_MODE, GL_LINEAR);
  setupFog();

  // Draw verylowres heightmap
  if (drawfog && drawterrain) {
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor3fv(this->skies->colorSet[FOG_COLOR]);
    //glColor3f(0,1,0);
    //glDisable(GL_FOG);
    const int lrr = 2;
    for (int i=cx-lrr; i<=cx+lrr; ++i) { //! \todo maybe broke this, investigate
      for (int j=cz-lrr; j<=cz+lrr; ++j) {
        //! \todo  some annoying visual artifacts when the verylowres terrain overlaps
        // maptiles that are close (1-off) - figure out how to fix.
        // still less annoying than hoels in the horizon when only 2-off verylowres tiles are drawn
        if ( !(i==cx&&j==cz) && mapIndex->oktile(i,j) && lowrestiles[j][i])
        {
          lowrestiles[j][i]->render();
        }
      }
    }
    //glEnable(GL_FOG);
  }

  // Draw height map
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL); // less z-fighting artifacts this way, I think
  glEnable(GL_LIGHTING);

  glEnable(GL_COLOR_MATERIAL);
  //glColorMaterial(GL_FRONT, GL_DIFFUSE);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glColor4f(1,1,1,1);
  // if we're using shaders let's give it some specular
  if (video.mSupportShaders) {
    Vec4D spec_color(0.1,0.1,0.1,0.1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 5);

    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  }

  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClientActiveTexture(GL_TEXTURE0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, detailtexcoords);
  glTexCoordPointer(2, GL_FLOAT, 0, 0);

  glClientActiveTexture(GL_TEXTURE1);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, alphatexcoords);
  glTexCoordPointer(2, GL_FLOAT, 0, 0);

  glClientActiveTexture(GL_TEXTURE0);

  OpenGL::SettingsSaver::save();

  // height map w/ a zillion texture passes
  //! \todo  Do we need to push the matrix here?

  glPushMatrix();

  if( drawterrain )
  {
    if(drawwireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if( mapIndex->tileLoaded( j, i ) )
        {
          mapIndex->getTile(j,i)->draw();
        }
      }
    }

    if(drawwireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  glPopMatrix();

  // Selection circle
  if( this->IsSelection( eEntry_MapChunk )  )
  {
    //int poly = this->GetCurrentSelectedTriangle();

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    //nameEntry * Selection = gWorld->GetCurrentSelection();

    //if( !Selection->data.mapchunk->strip )
    // Selection->data.mapchunk->initStrip();


    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );


    winX = (float)Environment::getInstance()->screenX;
    winY = (float)viewport[3] - (float)Environment::getInstance()->screenY;

    glReadPixels( Environment::getInstance()->screenX, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    Environment::getInstance()->Pos3DX = posX;
    Environment::getInstance()->Pos3DY = posY;
    Environment::getInstance()->Pos3DZ = posZ;

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glDisable(GL_CULL_FACE);
    //glDepthMask(false);
    //glDisable(GL_DEPTH_TEST);

    if(terrainMode == 0)
    {
      if(Environment::getInstance()->cursorType == 1)
        renderDisk_convenient(posX, posY, posZ, groundBrushRadius);
      else if(Environment::getInstance()->cursorType == 2)
        renderSphere_convenient(posX, posY, posZ, groundBrushRadius, 15);
    }
    else if(terrainMode == 1)
    {

      if(Environment::getInstance()->cursorType == 1)
        renderDisk_convenient(posX, posY, posZ, blurBrushRadius);
      else if(Environment::getInstance()->cursorType == 2)
        renderSphere_convenient(posX, posY, posZ, blurBrushRadius, 15);
    }
    else if(terrainMode == 2)
    {
      if(Environment::getInstance()->cursorType == 1)
        renderDisk_convenient(posX, posY, posZ, textureBrush.getRadius());
      else if(Environment::getInstance()->cursorType == 2)
        renderSphere_convenient(posX, posY, posZ, textureBrush.getRadius(), 15);
    }
    else renderSphere_convenient(posX, posY, posZ, 0.3f, 15);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    //GlDepthMask(true);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  }


  if (drawlines)
  {
    glDisable(GL_COLOR_MATERIAL);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setupFog();
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if(mapIndex->tileLoaded(j, i))
        {
          mapIndex->getTile(j,i)->drawLines();
          // mTiles[j][i].tile->drawMFBO();
        }
      }
    }
  }

  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);

  glColor4f(1,1,1,1);
  glEnable(GL_BLEND);

  if (video.mSupportShaders) {
    Vec4D spec_color(0,0,0,1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
  }

  // unbind hardware buffers
  glBindBuffer(GL_ARRAY_BUFFER, 0);




  glEnable(GL_CULL_FACE);

  glDisable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);

  // TEMP: for fucking around with lighting
  for(OpenGL::Light light = GL_LIGHT0; light < GL_LIGHT0 + 8; ++light )
  {
    glLightf(light, GL_CONSTANT_ATTENUATION, l_const);
    glLightf(light, GL_LINEAR_ATTENUATION, l_linear);
    glLightf(light, GL_QUADRATIC_ATTENUATION, l_quadratic);
  }





  // M2s / models
  if( drawmodels)
  {
    ModelManager::resetAnim();

    glEnable(GL_LIGHTING);  //! \todo  Is this needed? Or does this fuck something up?
    for( std::map<int, ModelInstance>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it )
      it->second.draw();

    //drawModelList();
  }




  // WMOs / map objects
  if(drawwmo || mapIndex->hasAGlobalWMO())
  {
    if (video.mSupportShaders)
    {
      Vec4D spec_color( 1.0f, 1.0f, 1.0f, 1.0f );
      glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
      glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 10 );

      glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );

      for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
        it->second.draw();

      spec_color = Vec4D( 0.0f, 0.0f, 0.0f, 1.0f );
      glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
      glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 0 );
    }
    else
    {
      for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
        it->second.draw();
    }
  }

  outdoorLights( true );
  setupFog();

  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
  glDisable(GL_CULL_FACE);

  glDisable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_LIGHTING);

  // gosh darn alpha blended evil

  OpenGL::SettingsSaver::restore();
  setupFog();

  glColor4f(1,1,1,1);
  glEnable(GL_BLEND);

  /*
  // temp frustum code
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBegin(GL_TRIANGLES);
  glColor4f(0,1,0,0.5);
  glVertex3fv(camera);
  glVertex3fv(fp - rt * fl * 1.33f - up * fl);
  glVertex3fv(fp + rt * fl * 1.33f - up * fl);
  glColor4f(0,0,1,0.5);
  glVertex3fv(camera);
  fl *= 0.5f;
  glVertex3fv(fp - rt * fl * 1.33f + up * fl);
  glVertex3fv(fp + rt * fl * 1.33f + up * fl);
  glEnd();
  */

  //glColor4f(1,1,1,1);
  //glDisable(GL_COLOR_MATERIAL);

  if(this->drawwater)
  {
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if(mapIndex->tileLoaded(j, i))
        {
          mapIndex->getTile(j,i)->drawWater();
        }
      }
    }
  }


  ex = camera.x / TILESIZE;
  ez = camera.z / TILESIZE;
}

static const GLuint MapObjName = 1;
static const GLuint DoodadName = 2;
static const GLuint MapTileName = 3;

void World::drawSelection( int cursorX, int cursorY, bool pOnlyMap )
{
  glSelectBuffer( sizeof( selectionBuffer ) / sizeof( GLuint ), selectionBuffer );
  glRenderMode( GL_SELECT );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );
  gluPickMatrix( cursorX, viewport[3] - cursorY, 7, 7, viewport );

  video.set3D_select();

  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  gluLookAt( camera.x, camera.y, camera.z, lookat.x, lookat.y, lookat.z, 0, 1, 0 );

  frustum.retrieve();

  glClear( GL_DEPTH_BUFFER_BIT );

  glInitNames();

  glPushName( MapTileName );
  if( drawterrain )
  {
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if(mapIndex->tileLoaded(j, i))
        {
          mapIndex->getTile(j,i)->drawSelect();
        }
      }
    }
  }
  glPopName();

  if( !pOnlyMap )
  {
    // WMOs / map objects
    if( drawwmo )
    {
      glPushName( MapObjName );
      glPushName( 0 );
      for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
      {
        it->second.drawSelect();
      }
      glPopName();
      glPopName();
    }

    // M2s / models
    if( drawmodels )
    {
      ModelManager::resetAnim();

      glPushName( DoodadName );
      glPushName( 0 );
      for( std::map<int, ModelInstance>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it )
      {
        it->second.drawSelect();
      }
      glPopName();
      glPopName();
    }
  }

  getSelection();
}

struct GLNameEntry
{
  GLuint stackSize;
  GLuint nearZ;
  GLuint farZ;
  struct
  {
    GLuint type;
    union
    {
      GLuint dummy;
      GLuint chunk;
    };
    union
    {
      GLuint uniqueId;
      GLuint triangle;
    };
  } stack;
};

void World::getSelection()
{
  GLuint minDist = 0xFFFFFFFF;
  GLNameEntry* minEntry = NULL;
  GLuint hits = glRenderMode( GL_RENDER );

  size_t offset = 0;

  //! \todo Isn't the closest one always the first? Iterating would be worthless then.
  while( hits-- > 0U )
  {
    GLNameEntry* entry = reinterpret_cast<GLNameEntry*>( &selectionBuffer[offset] );

    // We always push { MapObjName | DoodadName | MapTileName }, { 0, 0, MapTile }, { UID, UID, triangle }
    assert( entry->stackSize == 3 );

    if( entry->nearZ < minDist )
    {
      minDist = entry->nearZ;
      minEntry = entry;
    }

    offset += sizeof( GLNameEntry ) / sizeof( GLuint );
  }

  if( minEntry )
  {
    if( minEntry->stack.type == MapObjName || minEntry->stack.type == DoodadName )
    {
      mCurrentSelection = SelectionNames.findEntry( minEntry->stack.uniqueId );
    }
    else if( minEntry->stack.type == MapTileName )
    {
      mCurrentSelection = SelectionNames.findEntry( minEntry->stack.chunk );
      mCurrentSelectedTriangle = minEntry->stack.triangle;
    }
  }
}

void World::tick(float dt)
{
  mapIndex->enterTile(ex,ez);

  while (dt > 0.1f) {
    ModelManager::updateEmitters(0.1f);
    dt -= 0.1f;
  }
  ModelManager::updateEmitters(dt);
}

unsigned int World::getAreaID()
{
  const int mtx = camera.x / TILESIZE;
  const int mtz = camera.z / TILESIZE;
  const int mcx = fmod(camera.x, TILESIZE) / CHUNKSIZE;
  const int mcz = fmod(camera.z, TILESIZE) / CHUNKSIZE;

  //if((mtx<cx-1) || (mtx>cx+1) || (mtz<cz-1) || (mtz>cz+1))
  //  return 0;

  MapTile* curTile = mapIndex->getTile(mtz,mtx);
  if(!curTile) return 0;

  MapChunk *curChunk = curTile->getChunk(mcx, mcz);
  if(!curChunk)  return 0;

  return curChunk->getAreaID();
}

void World::clearHeight(int id, int x, int z)
{

  // set the Area ID on a tile x,z on all chunks
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      this->clearHeight(id, x, z, j, i);
    }
  }

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      // set the Area ID on a tile x,z on the chunk cx,cz
      MapTile *curTile;
      curTile = mapIndex->getTile(z,x);
      if(curTile == 0) return;
      mapIndex->setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->recalcNorms();
    }
  }

}

void World::clearHeight(int /*id*/, int x, int z , int _cx, int _cz)
{
  // set the Area ID on a tile x,z on the chunk cx,cz
  MapTile *curTile;
  curTile = mapIndex->getTile(z,x);
  if(curTile == 0) return;
  mapIndex->setChanged(z,x);
  MapChunk *curChunk = curTile->getChunk(_cx, _cz);
  if(curChunk == 0) return;

  curChunk->vmin.y = 9999999.0f;
  curChunk->vmax.y = -9999999.0f;
  curChunk->Changed=true;

  for(int i=0; i < mapbufsize; ++i)
  {
    curChunk->mVertices[i].y = 0.0f;

    curChunk->vmin.y = std::min(curChunk->vmin.y,curChunk-> mVertices[i].y);
    curChunk->vmax.y = std::max(curChunk->vmax.y, curChunk->mVertices[i].y);
  }

  glBindBuffer(GL_ARRAY_BUFFER, curChunk->vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(curChunk->mVertices), curChunk->mVertices, GL_STATIC_DRAW);
}

void World::clearAllModelsOnADT(int x,int z)
{
  // get the adt
  MapTile *curTile;
  curTile = mapIndex->getTile(z,x);
  if(curTile == 0) return;
  curTile->clearAllModels();
}

void World::deleteWaterLayer(int x,int z)
{
  MapTile *curTile = mapIndex->getTile(z,x);
  if(!curTile) return;

  curTile->Water->deleteLayer();
  mapIndex->setChanged(z,x);
}

void World::addWaterLayer(int x, int z)
{
  MapTile *curTile = mapIndex->getTile(z,x);
  if(!curTile) return;

  curTile->Water->addLayer(1, 255);
  mapIndex->setChanged(z,x);
}

void World::addWaterLayer(int x, int z, float height, unsigned char trans)
{
  MapTile *curTile = mapIndex->getTile(z,x);
  if(!curTile) return;

  curTile->Water->addLayer(height, trans);
  mapIndex->setChanged(z,x);
}

void World::setAreaID(int id, int x,int z)
{
  // set the Area ID on a tile x,z on all chunks
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      this->setAreaID(id, x, z, j, i);
    }
  }
}

void World::setAreaID(int id, int x, int z , int _cx, int _cz)
{

  // set the Area ID on a tile x,z on the chunk cx,cz
  MapTile *curTile;
  curTile = mapIndex->getTile(z,x);
  if(curTile == 0) return;
  mapIndex->setChanged(z,x);
  MapChunk *curChunk = curTile->getChunk(_cx, _cz);

  if(curChunk == 0) return;

  curChunk->setAreaID(id);
}

void World::drawTileMode(float /*ah*/)
{
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPushMatrix();
  glScalef(zoom,zoom,1.0f);

  glPushMatrix();
  glTranslatef(-camera.x/CHUNKSIZE,-camera.z/CHUNKSIZE,0);

  minX = camera.x/CHUNKSIZE - 2.0f*video.ratio()/zoom;
  maxX = camera.x/CHUNKSIZE + 2.0f*video.ratio()/zoom;
  minY = camera.z/CHUNKSIZE - 2.0f/zoom;
  maxY = camera.z/CHUNKSIZE + 2.0f/zoom;

  glEnableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);

  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if(mapIndex->tileLoaded(j,i))
      {
        mapIndex->getTile(j,i)->drawTextures();
      }
    }
  }

  glDisableClientState(GL_COLOR_ARRAY);

  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);



  glPopMatrix();
  if (drawlines) {
    glTranslatef(fmod(-camera.x/CHUNKSIZE,16), fmod(-camera.z/CHUNKSIZE,16),0);
    /*  for(int x=-32;x<=48;x++)
    {
      if(x%16==0)
        glColor4f(0.0f,1.0f,0.0f,0.5f);
      else
        glColor4f(1.0f,0.0f,0.0f,0.5f);
      glBegin(GL_LINES);
      glVertex3f(-32.0f,(float)x,-1);
      glVertex3f(48.0f,(float)x,-1);
      glVertex3f((float)x,-32.0f,-1);
      glVertex3f((float)x,48.0f,-1);
      glEnd();
    }*/

    for(float x = -32.0f; x <= 48.0f; x += 1.0f)
    {
      if( static_cast<int>(x) % 16 )
        glColor4f(1.0f,0.0f,0.0f,0.5f);
      else
        glColor4f(0.0f,1.0f,0.0f,0.5f);
      glBegin(GL_LINES);
      glVertex3f(-32.0f,x,-1);
      glVertex3f(48.0f,x,-1);
      glVertex3f(x,-32.0f,-1);
      glVertex3f(x,48.0f,-1);
      glEnd();
    }
  }

  glPopMatrix();

  ex = camera.x / TILESIZE;
  ez = camera.z / TILESIZE;
}

bool World::GetVertex(float x,float z, Vec3D *V)
{
  const int newX = x / TILESIZE;
  const int newZ = z / TILESIZE;

  if( !mapIndex->tileLoaded( newZ, newX ) )
  {
    return false;
  }

  return mapIndex->getTile(newZ,newX)->GetVertex(x, z, V);
}

void World::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if(mapIndex->getTile(j,i)->getChunk(ty,tx)->changeTerrain(x,z,change,radius,BrushType))
              mapIndex->setChanged(j, i);
          }
        }
      }
    }
  }

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            mapIndex->getTile(j,i)->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}

void World::flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType)
{
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( mapIndex->getTile(j,i)->getChunk(ty,tx)->flattenTerrain(x,z,h,remain,radius,BrushType) )
              mapIndex->setChanged(j,i);
          }
        }
      }
    }
  }

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            mapIndex->getTile(j,i)->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}

void World::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if(mapIndex->getTile(j,i)->getChunk(ty,tx)->blurTerrain(x, z, remain, radius, BrushType))
              mapIndex->setChanged(j,i);
          }
        }
      }
    }
  }

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            mapIndex->getTile(j,i)->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}

bool World::paintTexture(float x, float z, Brush *brush, float strength, float pressure, OpenGL::Texture* texture)
{
  const int xLower = (int)((x - brush->getRadius())/ TILESIZE);
  const int xUper = (int)((x + brush->getRadius())/ TILESIZE)+1;
  const int zLower = (int)((z - brush->getRadius()) / TILESIZE);
  const int zUper = (int)((z + brush->getRadius()) / TILESIZE)+1;
  bool succ = false;

  for( int j = zLower; j < zUper; ++j )
  {
    for( int i = xLower; i < xUper; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        int chunkLowerX = (int)((x - brush->getRadius())/ CHUNKSIZE)-i*16;
        int chunkUperX = (int)((x + brush->getRadius())/ CHUNKSIZE)-i*16+1;
        int chunkLowerZ = (int)((z - brush->getRadius()) / CHUNKSIZE)-j*16;
        int chunkUperZ = (int)((z + brush->getRadius()) / CHUNKSIZE)-j*16+1;

        if(chunkLowerX < 0) chunkLowerX = 0;
        if(chunkLowerZ < 0) chunkLowerZ = 0;
        if(chunkUperX > 16) chunkUperX = 16;
        if(chunkUperZ > 16) chunkUperZ = 16;

        for( size_t ty = chunkLowerX; ty < chunkUperX; ++ty )
        {
          for( size_t tx = chunkLowerZ; tx < chunkUperZ; ++tx )
          {
            if(mapIndex->getTile(j,i)->getChunk( ty, tx )->paintTexture( x, z, brush, strength, pressure, texture))
            {
              succ |= true;
              mapIndex->setChanged( j, i );
            }
          }
        }
      }
    }
  }

  return succ;
}

void World::eraseTextures(float x, float z)
{
  mapIndex->setChanged(x,z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;

  LogDebug << "Erasing Textures at " << x << " and " << z << std::endl;

  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mapIndex->getTile(j,i)->getChunk(ty, tx);
            if(chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z)
            {
              chunk->eraseTextures();
            }
          }
        }
      }
    }
  }
}

void World::overwriteTextureAtCurrentChunk(float x, float z, OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture)
{
  mapIndex->setChanged(x,z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;

  LogDebug << "Switching Textures at " << x << " and " << z << std::endl;

  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mapIndex->getTile(j,i)->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              chunk->switchTexture(oldTexture, newTexture);
            }
          }
        }
      }
    }
  }
}

void World::addHole( float x, float z, bool big )
{
  mapIndex->setChanged(x, z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;

  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mapIndex->getTile(j,i)->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
              int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
              if(big)
                chunk->addHoleBig( k, l );
              else
                chunk->addHole( k, l );
            }
          }
        }
      }
    }
  }
}

void World::removeHole( float x, float z,bool big )
{
  mapIndex->setChanged(x, z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;

  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( mapIndex->tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mapIndex->getTile(j,i)->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
              int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
              if(big)
                chunk->removeHoleBig( k, l );
              else
                chunk->removeHole( k, l );
            }
          }
        }
      }
    }
  }
}

void World::jumpToCords(Vec3D pos)
{
  this->camera = pos;
}

void World::saveMap()
{
  //! \todo  Output as BLP.
  unsigned char image[256*256*3];
  MapTile *ATile;
  FILE *fid;
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glReadBuffer(GL_BACK);

  minX=-64*16;
  maxX=64*16;
  minY=-64*16;
  maxY=64*16;

  glEnableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  for(int y=0;y<64;y++)
  {
    for(int x=0;x<64;x++)
    {
      if(!(mapIndex->getFlag(y,x) & 1))
        continue;

      ATile=mapIndex->loadTile(x,y);
      glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

      glPushMatrix();
      glScalef(0.08333333f,0.08333333f,1.0f);

      //glTranslatef(-camera.x/CHUNKSIZE,-camera.z/CHUNKSIZE,0);
      glTranslatef( x * -16.0f - 8.0f, y * -16.0f - 8.0f, 0.0f );

      ATile->drawTextures();
      glPopMatrix();
      glReadPixels(video.xres()/2-128,video.yres()/2-128,256,256,GL_RGB,GL_UNSIGNED_BYTE,image);
      video.flip();
      std::stringstream ss;
      ss << basename.c_str() << "_map_" << x << "_" << y << ".raw";
      fid=fopen(ss.str().c_str(),"wb");
      fwrite(image,256*3,256,fid);
      fclose(fid);
    }
  }

  glDisableClientState(GL_COLOR_ARRAY);

  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void World::deleteModelInstance( int pUniqueID )
{
  std::map<int, ModelInstance>::iterator it = mModelInstances.find( pUniqueID );
  mapIndex->setChanged( it->second.pos.x, it->second.pos.z );
  mModelInstances.erase( it );
  ResetSelection();
}

void World::deleteWMOInstance( int pUniqueID )
{
  std::map<int, WMOInstance>::iterator it = mWMOInstances.find( pUniqueID );
  mapIndex->setChanged( it->second.pos.x, it->second.pos.z );
  mWMOInstances.erase( it );
  ResetSelection();
}

void World::addModel( nameEntry entry, Vec3D newPos,bool copyit  )
{
  if( entry.type == eEntry_Model )
    this->addM2( entry.data.model->model, newPos,copyit );
  else if( entry.type == eEntry_WMO )
    this->addWMO( entry.data.wmo->wmo, newPos,copyit );
}

void World::addM2( Model *model, Vec3D newPos, bool copyit)
{
  int temp = 0;
  if  (mModelInstances.empty()) {
    temp = 0;
  }
  else{
    temp = mModelInstances.rbegin()->first + 1;
  }
  const int lMaxUID = temp;
  //  ( ( mModelInstances.empty() ? 0 : mModelInstances.rbegin()->first + 1 ),
  //                           ( mWMOInstances.empty() ? 0 : mWMOInstances.rbegin()->first + 1 ) );

  ModelInstance newModelis = ModelInstance();
  newModelis.model = model;
  newModelis.nameID = -1;
  newModelis.d1 = lMaxUID;
  newModelis.pos = newPos;
  newModelis.sc = 1;

  if(Settings::getInstance()->copyModelStats
     && copyit
     && Environment::getInstance()->get_clipboard().type == eEntry_Model)
  {
    // copy rot size from original model. Dirty but woring
    newModelis.sc = Environment::getInstance()->get_clipboard().data.model->sc;
    newModelis.dir = Environment::getInstance()->get_clipboard().data.model->dir;
    newModelis.ldir = Environment::getInstance()->get_clipboard().data.model->ldir;
  }

  if(Settings::getInstance()->copy_rot)
  {
    newModelis.dir.y += (rand() % 360 + 1);
  }

  if(Settings::getInstance()->copy_tile)
  {
    newModelis.dir.x += (rand() % 5 + 1);
    newModelis.dir.z += (rand() % 5 + 1);
  }

  if(Settings::getInstance()->copy_size)
  {
    newModelis.sc *= misc::randfloat( 0.9f, 1.1f );
  }



  mModelInstances.insert( std::pair<int,ModelInstance>( lMaxUID, newModelis ));
  mapIndex->setChanged(newPos.x,newPos.z);
}

void World::addWMO( WMO *wmo, Vec3D newPos, bool copyit )
{

  const int lMaxUID = std::max( ( mModelInstances.empty() ? 0 : mModelInstances.rbegin()->first + 1 ),
                                ( mWMOInstances.empty() ? 0 : mWMOInstances.rbegin()->first + 1 ) ) ;

  WMOInstance newWMOis(wmo);
  newWMOis.pos = newPos;
  newWMOis.mUniqueID = lMaxUID;

  if(Settings::getInstance()->copyModelStats
     && Environment::getInstance()->get_clipboard().type == eEntry_WMO)
  {
    // copy rot from original model. Dirty but working
    newWMOis.dir = Environment::getInstance()->get_clipboard().data.wmo->dir;
  }

  // recalc the extends
  newWMOis.recalcExtents();

  mWMOInstances.insert( std::pair<int,WMOInstance>( lMaxUID, newWMOis ));
  mapIndex->setChanged(newPos.x,newPos.z);

}

unsigned int World::getMapID()
{
  return this->mMapId;
}


void World::moveHeight(int id, int x, int z)
{

  // set the Area ID on a tile x,z on all chunks
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      this->moveHeight(id, x, z, j, i);
    }
  }

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      // set the Area ID on a tile x,z on the chunk cx,cz
      MapTile *curTile;
      curTile = mapIndex->getTile(z,x);
      if(curTile == 0) return;
      mapIndex->setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->recalcNorms();
    }
  }

}

void World::moveHeight(int /*id*/, int x, int z , int _cx, int _cz)
{
  // set the Area ID on a tile x,z on the chunk cx,cz
  MapTile *curTile;
  curTile = mapIndex->getTile(z,x);
  if(curTile == 0) return;
  mapIndex->setChanged(z,x);
  MapChunk *curChunk = curTile->getChunk(_cx, _cz);
  if(curChunk == 0) return;

  curChunk->vmin.y = 9999999.0f;
  curChunk->vmax.y = -9999999.0f;
  curChunk->Changed = true;

  float heightDelta = 0.0f;
  nameEntry *selection = gWorld->GetCurrentSelection();

  if(selection)
    if(selection->type == eEntry_MapChunk)
    {
      // chunk selected
      heightDelta = gWorld->camera.y - selection->data.mapchunk->py;
    }

  if( heightDelta * heightDelta <= 0.1f ) return;

  for(int i=0; i < mapbufsize; ++i)
  {
    curChunk->mVertices[i].y = curChunk->mVertices[i].y + heightDelta;

    curChunk->vmin.y = std::min(curChunk->vmin.y,curChunk-> mVertices[i].y);
    curChunk->vmax.y = std::max(curChunk->vmax.y, curChunk->mVertices[i].y);
  }

  glBindBuffer(GL_ARRAY_BUFFER, curChunk->vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(curChunk->mVertices), curChunk->mVertices, GL_STATIC_DRAW);


}

void World::setBaseTexture( int x, int z )
{
  if( !UITexturingGUI::getSelectedTexture() ) return;
  MapTile *curTile;
  curTile = mapIndex->getTile(z,x);
  if(curTile == 0) return;

  // clear all textures on the adt and set selected texture as base texture
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->eraseTextures();
      curChunk->addTexture( UITexturingGUI::getSelectedTexture() );
      UITexturingGUI::getSelectedTexture()->addReference();
    }
  }
}

void World::swapTexture( int x, int z, OpenGL::Texture *tex )
{
  if( !UITexturingGUI::getSelectedTexture() ) return;
  MapTile *curTile;
  curTile = mapIndex->getTile(z,x);
  if(curTile == 0) return;

  // clear all textures on the adt and set selected texture as base texture
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->switchTexture(tex,UITexturingGUI::getSelectedTexture());
    }
  }
}

void World::saveWDT()
{
  // int lCurrentPosition = 0;
  //sExtendableArray lWDTFile = sExtendableArray();
  // lWDTFile.Extend( 8 + 0x4 );
  // SetChunkHeader( lWDTFile, lCurrentPosition, 'MPHD', 4 );

  // MPQFile f( "test.WDT" );
  // f.setBuffer( lWDTFile.GetPointer<uint8_t>(), lWDTFile.mSize );
  // f.SaveFile();
  // f.close();
}

bool World::canWaterSave(int x, int y)
{ 

  if(!mapIndex->tileLoaded(y, x)) //! \todo else there are null pointers
    return false;

  return mapIndex->getTile(y, x)->canWaterSave();
}

void World::setWaterHeight(int x, int y, float h)
{ 
  if(mapIndex->tileLoaded(y, x))
  {
    mapIndex->getTile(y,x)->Water->setHeight(h);
    mapIndex->setChanged(y,x);
  }
}

float World::getWaterHeight(int x, int y)
{ 
    if(mapIndex->tileLoaded(y, x))
    {
      return mapIndex->getTile(y,x)->Water->getHeight();
    }
    else return 0;
}

void World::setWaterTrans(int x, int y, unsigned char value)
{ 

  if(mapIndex->tileLoaded(y, x))
  {
    mapIndex->getTile(y,x)->Water->setTrans(value);
    mapIndex->setChanged(y,x);
  }
}

unsigned char World::getWaterTrans(int x, int y)
{ 
  if(mapIndex->tileLoaded(y, x))
  {
    return mapIndex->getTile(y,x)->Water->getOpacity();
  }
  else return 255;
}

void World::setWaterType(int x, int y, int type)
{ 

  if(mapIndex->tileLoaded(y, x))
  {
    mapIndex->getTile(y,x)->Water->setType(type);
    mapIndex->setChanged(y,x);
  }

}

int World::getWaterType(int x, int y)
{ 
  if(mapIndex->tileLoaded(y, x))
  {
    return mapIndex->getTile(y,x)->Water->getType();
    //mapIndex->setChanged(y,x); Should only change if you change something :)
  }
  else return false;
}

void World::autoGenWaterTrans(int x, int y, int factor)
{
  if(mapIndex->tileLoaded(y, x))
  {
    mapIndex->getTile(y,x)->Water->autoGen(factor);
    mapIndex->setChanged(y,x);
  }
}

