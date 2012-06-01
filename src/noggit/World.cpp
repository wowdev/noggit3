// World.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/World.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <time.h>
#include <stdio.h> //needed on linux?


#include <QSettings>

#include <math/bounded_nearest.h>
#include <math/random.h>
#include <math/vector_2d.h>

#include <opengl/call_list.h>
#include <opengl/settings_saver.h>
#include <opengl/scoped.h>

#include <noggit/blp_texture.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h>
#include <noggit/UITexturingGUI.h>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/MapTile.h>
#include <noggit/mpq/file.h>

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
  //the same quadric can be re-used for drawing many objects
  glDisable(GL_LIGHTING);

  //! \todo This should be passed in!
  QSettings settings;
  glColor4f ( settings.value ("cursor/red", 1.0f).toFloat()
            , settings.value ("cursor/green", 1.0f).toFloat()
            , settings.value ("cursor/blue", 1.0f).toFloat()
            , settings.value ("cursor/alpha", 1.0f).toFloat()
            );

  GLUquadricObj *quadric=gluNewQuadric();
  gluQuadricNormals(quadric, GLU_SMOOTH);
  renderSphere(x,y,z,x,y,z,0.3f,15,quadric);
  renderSphere(x,y,z,x,y,z,radius,subdivisions,quadric);
  gluDeleteQuadric(quadric);
  glEnable(GL_LIGHTING);
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

  glPushMatrix();
  glDisable(GL_DEPTH_TEST);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  //draw the quadric
  glTranslatef(x1, y1, z1);
  glRotatef(ax, rx, ry, 0.0f);
  glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

  //! \todo This should be passed in!
  QSettings settings;
  glColor4f ( settings.value ("cursor/red", 1.0f).toFloat()
            , settings.value ("cursor/green", 1.0f).toFloat()
            , settings.value ("cursor/blue", 1.0f).toFloat()
            , settings.value ("cursor/alpha", 1.0f).toFloat()
            );

  gluQuadricOrientation(quadric, GLU_OUTSIDE);
  gluDisk(quadric, radius , radius + 1.0f, subdivisions, 1);

  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
}

void renderDisk_convenient(float x, float y, float z, float radius)
{

  int subdivisions =(int)radius * 2.0;

  if( subdivisions < 20 ) subdivisions=20;
  glDisable(GL_LIGHTING);
  GLUquadricObj *quadric = gluNewQuadric();
  gluQuadricDrawStyle(quadric, GLU_LINE);
  gluQuadricNormals(quadric, GLU_SMOOTH);
  renderDisk(x, y, z, x, y, z, radius, subdivisions, quadric);
  renderSphere(x,y,z,x,y,z,0.3f,15,quadric);
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

  const QString filename
    ( QString ("World\\Maps\\%1\\%1.wdt")
      .arg (QString::fromStdString (lMapName))
    );

  if (!noggit::mpq::file::exists (filename))
  {
    Log << "World " << pMapId << ": " << lMapName << " has no WDT file!" << std::endl;
    return false;
  }

  noggit::mpq::file mf (filename);

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
  : cx( -1 )
  , cz( -1 )
  , ex( -1 )
  , ez( -1 )
  , mCurrentSelection( NULL )
  , mCurrentSelectedTriangle( 0 )
  , SelectionMode( false )
  , mBigAlpha( false )
  , mWmoFilename( "" )
  , mWmoEntry( ENTRY_MODF() )
  , detailtexcoords( 0 )
  , alphatexcoords( 0 )
  , mMapId( 0xFFFFFFFF )
  , ol( NULL )
  , l_const( 0.0f )
  , l_linear( 0.7f )
  , l_quadratic( 0.03f )
  , animtime( 0 )
  , time( 1450 )
  , basename( name )
  , fogdistance( 777.0f )
  , culldistance( fogdistance )
  , skies( NULL )
  , mHasAGlobalWMO( false )
  , noadt( false )
  , outdoorLightStats( OutdoorLightStats() )
  , camera( ::math::vector_3d( 0.0f, 0.0f, 0.0f ) )
  , lookat( ::math::vector_3d( 0.0f, 0.0f, 0.0f ) )
  , frustum( Frustum() )
  , _selection_names (this)
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

  const QString filename
    ( QString ("World\\Maps\\%1\\%1.wdt")
      .arg (QString::fromStdString (basename))
    );

  noggit::mpq::file theFile (filename);
  uint32_t fourcc;
  //uint32_t size;

  // - MVER ----------------------------------------------

  uint32_t version;

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );
  theFile.read( &version, 4 );

  //! \todo find the correct version of WDT files.
  assert( fourcc == 'MVER' && version == 18 );

  // - MHDR ----------------------------------------------

  uint32_t flags;

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MPHD' );

  theFile.read( &flags, 4 );
  theFile.seekRelative( 4 * 7 );

  mHasAGlobalWMO = flags & 1;
  mBigAlpha = flags & 4;

  // - MAIN ----------------------------------------------

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MAIN' );

  /// this is the theory. Sadly, we are also compiling on 64 bit machines with size_t being 8 byte, not 4. Therefore, we can't do the same thing, Blizzard does in its 32bit executable.
  //theFile.read( &(mTiles[0][0]), sizeof( 8 * 64 * 64 ) );

  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      theFile.read( &mTiles[j][i].flags, 4 );
      theFile.seekRelative( 4 );
      mTiles[j][i].tile = NULL;
    }
  }

  if( !theFile.is_at_end_of_file() )
  {
    //! \note We actually don't load WMO only worlds, so we just stop reading here, k?
    //! \bug MODF reads wrong. The assertion fails every time. Somehow, it keeps being MWMO. Or are there two blocks?

    mHasAGlobalWMO = false;

#ifdef __ASSERTIONBUGFIXED

    // - MWMO ----------------------------------------------

    theFile.read( &fourcc, 4 );
    theFile.read( &size, 4 );

    assert( fourcc == 'MWMO' );

    char * wmoFilenameBuf = new char[size];
    theFile.read( &wmoFilenameBuf, size );

    mWmoFilename = wmoFilenameBuf;

    free(wmoFilenameBuf);

    // - MODF ----------------------------------------------

    theFile.read( &fourcc, 4 );
    theFile.seekRelative( 4 );

    assert( fourcc == 'MODF' );

    theFile.read( &mWmoEntry, sizeof( ENTRY_MODF ) );

#endif //__ASSERTIONBUGFIXED

  }

  // -----------------------------------------------------

  theFile.close();

  if( !mHasAGlobalWMO )
    initMinimap();

  // don't load map objects while still on the menu screen
  //initDisplay();
}

static inline QRgb color_for_height (int16_t height)
{
  struct ranged_color
  {
    const QRgb color;
    const int16_t start;
    const int16_t stop;

    ranged_color ( const QRgb& color_
                 , const int16_t& start_
                 , const int16_t& stop_
                 )
    : color (color_)
    , start (start_)
    , stop (stop_)
    { }
  };

  static const ranged_color colors[] =
    { ranged_color (qRgb (20, 149, 7), 0, 600)
    , ranged_color (qRgb (137, 84, 21), 600, 1200)
    , ranged_color (qRgb (96, 96, 96), 1200, 1600)
    , ranged_color (qRgb (255, 255, 255), 1600, 0x7FFF)
    };
  static const size_t num_colors (sizeof (colors) / sizeof (ranged_color));

  if (height < colors[0].start)
  {
    return qRgb (0, 0, 255 + qMax (height / 2.0, -255.0));
  }
  else if (height >= colors[num_colors - 1].stop)
  {
    return colors[num_colors].color;
  }

  qreal t (1.0);
  size_t correct_color (num_colors - 1);

  for (size_t i (0); i < num_colors - 1; ++i)
  {
    if (height >= colors[i].start && height < colors[i].stop)
    {
      t = (height - colors[i].start) / qreal (colors[i].stop - colors[i].start);
      correct_color = i;
      break;
    }
  }

  return qRgb ( qRed (colors[correct_color + 1].color) * t + qRed (colors[correct_color].color) * (1.0 - t)
              , qGreen (colors[correct_color + 1].color) * t + qGreen (colors[correct_color].color) * (1.0 - t)
              , qBlue (colors[correct_color + 1].color) * t + qBlue (colors[correct_color].color) * (1.0 - t)
              );
}


void World::initMinimap()
{

  // init the minimap image

  _minimap = QImage (17 * 64, 17 * 64, QImage::Format_RGB32);

  _minimap.fill (Qt::transparent);

  const QString filename
    ( QString ("World\\Maps\\%1\\%1.wdl")
      .arg (QString::fromStdString (basename))
    );

  // if wdl do not exist return.

  if( noggit::mpq::file::exists(filename) == false ) return;

  noggit::mpq::file wdl_file (filename);

  uint32_t fourcc;
  uint32_t size;

  // - MVER ----------------------------------------------

  uint32_t version;

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);
  wdl_file.read (&version, 4);

  assert (fourcc == 'MVER' && size == 4 && version == 18);

  // - MWMO ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MWMO');
      // Filenames for WMO that appear in the low resolution map. Zero terminated strings.

  wdl_file.seekRelative (size);

  // - MWID ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MWID');
      // List of indexes into the MWMO chunk.

  wdl_file.seekRelative (size);

  // - MODF ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MODF');
      // Placement information for the WMO. Appears to be the same 64 byte structure used in the WDT and ADT MODF chunks.

  wdl_file.seekRelative (size);

  // - MAOF ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MAOF' && size == 64 * 64 * sizeof (uint32_t));

  uint32_t mare_offsets[64][64];
  wdl_file.read (mare_offsets, 64 * 64 * sizeof (uint32_t));

  // - MARE and MAHO by offset ---------------------------

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (mare_offsets[y][x])
      {
        wdl_file.seek (mare_offsets[y][x]);
        wdl_file.read (&fourcc, 4);
        wdl_file.read (&size, 4);

        assert (fourcc == 'MARE');
        assert (size == 0x442);

        //! \todo There also is a second heightmap appended which has additional 16*16 pixels.
        //! \todo There also is MAHO giving holes into this heightmap.

        const int16_t* data (wdl_file.get<int16_t> (mare_offsets[y][x] + 8));

        for (size_t j (0); j < 17; ++j)
        {
          for (size_t i (0); i < 17; ++i)
          {
            _minimap.setPixel (x * 17 + i, y * 17 + j, color_for_height (data[j * 17 + i]));
          }
        }
      }
    }
  }
}

void World::initLowresTerrain()
{
  const QString filename
    ( QString ("World\\Maps\\%1\\%1.wdl")
      .arg (QString::fromStdString (basename))
    );

  // if wdl do not exist return.

  if( noggit::mpq::file::exists(filename) == false ) return;

  noggit::mpq::file wdl_file (filename);

  uint32_t fourcc;
  uint32_t size;

  // - MVER ----------------------------------------------

  uint32_t version;

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);
  wdl_file.read (&version, 4);

  assert (fourcc == 'MVER' && size == 4 && version == 18);

  // - MWMO ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MWMO');
  // Filenames for WMO that appear in the low resolution map. Zero terminated strings.

  wdl_file.seekRelative (size);

  // - MWID ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MWID');
  // List of indexes into the MWMO chunk.

  wdl_file.seekRelative (size);

  // - MODF ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MODF');
  // Placement information for the WMO. Appears to be the same 64 byte structure used in the WDT and ADT MODF chunks.

  wdl_file.seekRelative (size);

  // - MAOF ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MAOF' && size == 64 * 64 * sizeof (uint32_t));

  uint32_t mare_offsets[64][64];
  wdl_file.read (mare_offsets, 64 * 64 * sizeof (uint32_t));

  // - MARE and MAHO by offset ---------------------------

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (mare_offsets[y][x])
      {
        wdl_file.seek (mare_offsets[y][x]);
        wdl_file.read (&fourcc, 4);
        wdl_file.read (&size, 4);

        assert (fourcc == 'MARE' && size == 0x442);

        ::math::vector_3d vertices_17[17][17];
        ::math::vector_3d vertices_16[16][16];

        const int16_t* data_17 (wdl_file.get<int16_t> (mare_offsets[y][x] + 8));

        for (size_t j (0); j < 17; ++j)
        {
          for (size_t i (0); i < 17; ++i)
          {
            vertices_17[j][i] = ::math::vector_3d ( TILESIZE * (x + i / 16.0f)
                                      , data_17[j * 17 + i]
                                      , TILESIZE * (y + j / 16.0f)
                                      );
          }
        }

        const int16_t* data_16 (wdl_file.get<int16_t> (mare_offsets[y][x] + 8 + 17 * 17 * sizeof (int16_t)));

        for (size_t j (0); j < 16; ++j)
        {
          for (size_t i (0); i < 16; ++i)
          {
            vertices_16[j][i] = ::math::vector_3d ( TILESIZE * (x + (i + 0.5f) / 16.0f)
                                      , data_16[j * 16 + i]
                                      , TILESIZE * (y + (j + 0.5f) / 16.0f)
                                      );
          }
        }

        lowrestiles[y][x] = new opengl::call_list;
        lowrestiles[y][x]->start_recording();

        //! \todo Make a strip out of this.
        glBegin( GL_TRIANGLES );
        for (size_t j (0); j < 16; ++j )
        {
          for (size_t i (0); i < 16; ++i )
          {
            glVertex3fv (vertices_17[j][i]);
            glVertex3fv (vertices_16[j][i]);
            glVertex3fv (vertices_17[j][i + 1]);
            glVertex3fv (vertices_17[j][i + 1]);
            glVertex3fv (vertices_16[j][i]);
            glVertex3fv (vertices_17[j + 1][i + 1]);
            glVertex3fv (vertices_17[j + 1][i + 1]);
            glVertex3fv (vertices_16[j][i]);
            glVertex3fv (vertices_17[j + 1][i]);
            glVertex3fv (vertices_17[j + 1][i]);
            glVertex3fv (vertices_16[j][i]);
            glVertex3fv (vertices_17[j][i]);
          }
        }
        glEnd();

        lowrestiles[y][x]->end_recording();

        //! \todo There also is MAHO giving holes into this heightmap.
      }
    }
  }
}

void initGlobalVBOs( GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords )
{
  if( !*pDetailTexCoords && !*pAlphaTexCoords )
  {
    ::math::vector_2d temp[mapbufsize], *vt;
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
        *vt++ = ::math::vector_2d(tx, ty);
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
        *vt++ = ::math::vector_2d (tx, ty);
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
  initGlobalVBOs( &detailtexcoords, &alphatexcoords );

  noadt = false;

  if( mHasAGlobalWMO )
  {
    WMOInstance *inst = new WMOInstance( this, WMOManager::add( this, mWmoFilename ), &mWmoEntry );

    mWMOInstances.insert( std::pair<int,WMOInstance *>( mWmoEntry.uniqueID, inst ) );
    camera = inst->pos;
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
      if( tileLoaded( j, i ) )
      {
        delete mTiles[j][i].tile;
        mTiles[j][i].tile = NULL;
      }
    }
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

  LogDebug << "Unloaded world \"" << basename << "\"." << std::endl;
}

inline bool oktile( int z, int x )
{
  return !( z < 0 || x < 0 || z > 64 || x > 64 );
}

bool World::hasTile( int pZ, int pX ) const
{
  return oktile( pZ, pX ) && ( mTiles[pZ][pX].flags & 1 );
}

void World::enterTile( int x, int z )
{
  if( !hasTile( z, x ) )
  {
    noadt = true;
    return;
  }

  noadt = false;

  cx = x;
  cz = z;
  for( int i = std::max(cz - 2, 0); i < std::min(cz + 2, 64); ++i )
  {
    for( int j = std::max(cx - 2, 0); j < std::min(cx + 2, 64); ++j )
    {
      mTiles[i][j].tile = loadTile( i, j );
    }
  }
}

void World::set_camera_above_terrain()
{
  if (!tileLoaded (cz, cx))
  {
    return;
  }

  camera.y (qMax (mTiles[cz][cx].tile->getMaxHeight(), 0.0f) + 50.0f);
}

void World::reloadTile(int x, int z)
{
  if( tileLoaded( z, x ) )
  {
    delete mTiles[z][x].tile;
    mTiles[z][x].tile = NULL;

    std::stringstream filename;
    filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

    mTiles[z][x].tile = new MapTile( this, x, z, filename.str(), mBigAlpha );
    enterTile( cx, cz );
  }
}

void World::saveTile(int x, int z)
{
  // save goven tile
  if( tileLoaded( z, x ) )
  {
    mTiles[z][x].tile->saveTile ( mModelInstances.begin()
                                , mModelInstances.end()
                                , mWMOInstances.begin()
                                , mWMOInstances.end()
                                );
  }
}

void World::saveTileCata(int x, int z)
{
  if( tileLoaded( z, x ) )
  {
    mTiles[z][x].tile->saveTileCata ( mModelInstances.begin()
                                    , mModelInstances.end()
                                    , mWMOInstances.begin()
                                    , mWMOInstances.end()
                                    );
  }
}

void World::saveChanged()
{
  // save all changed tiles
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        if(getChanged(j,i))
        {
          mTiles[j][i].tile->saveTile ( mModelInstances.begin()
                                      , mModelInstances.end()
                                      , mWMOInstances.begin()
                                      , mWMOInstances.end()
                                      );
          unsetChanged(j,i);
        }
      }
    }
  }

}

inline bool World::tileLoaded(int z, int x)
{
  return hasTile( z, x ) && mTiles[z][x].tile;
}

MapTile* World::loadTile(int z, int x)
{
  if( !hasTile( z, x ) )
  {
    return NULL;
  }

  if( tileLoaded( z, x ) )
  {
    return mTiles[z][x].tile;
  }

  const QString filename
    ( QString ("World\\Maps\\%1\\%1_%2_%3.adt")
      .arg (QString::fromStdString (basename))
      .arg (x)
      .arg (z)
    );

  if( !noggit::mpq::file::exists( filename ) )
  {
    LogError << "The requested tile \"" << qPrintable (filename) << "\" does not exist! Oo" << std::endl;
    return NULL;
  }

  mTiles[z][x].tile = new MapTile( this, x, z, filename.toStdString(), mBigAlpha );
  return mTiles[z][x].tile;
}



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

/*
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
  ::math::vector_4d black(0,0,0,0);
  ::math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  float di = outdoorLightStats.dayIntensity;
  //float ni = outdoorLightStats.nightIntensity;

  ::math::vector_3d dd = outdoorLightStats.dayDir;
  // HACK: let's just keep the light source in place for now
  //::math::vector_4d pos(-1, 1, -1, 0);
  ::math::vector_4d pos(-dd.x(), -dd.z(), dd.y(), 0.0f);
  ::math::vector_4d col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1.0f);
  glLightfv(GL_LIGHT0, GL_AMBIENT, black);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  /*
  dd = outdoorLightStats.nightDir;
  pos(-dd.x(), -dd.z(), dd.y(), 0.0f);
  col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1.0f);
  glLightfv(GL_LIGHT1, GL_AMBIENT, black);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT1, GL_POSITION, pos);*/
}

/*void World::outdoorLighting2()
{
  ::math::vector_4d black(0,0,0,0);
  ::math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  float di = outdoorLightStats.dayIntensity, ni = outdoorLightStats.nightIntensity;
  di = 1;
  ni = 0;

  //::math::vector_3d dd = outdoorLightStats.dayDir;
  // HACK: let's just keep the light source in place for now
  ::math::vector_4d pos(-1, -1, -1, 0);
  ::math::vector_4d col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1);
  glLightfv(GL_LIGHT0, GL_AMBIENT, black);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  */
  /*
  ::math::vector_3d dd = outdoorLightStats.nightDir;
  ::math::vector_4d pos(-dd.x(), -dd.z(), dd.y(), 0);
  ::math::vector_4d col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1);
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
    ::math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
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
    ::math::vector_4d ambient(0, 0, 0, 1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
  }
}

void World::setupFog (bool draw_fog)
{
  if (draw_fog)
  {

    //float fogdist = 357.0f; // minimum draw distance in wow
    //float fogdist = 777.0f; // maximum draw distance in wow

    float fogdist = fogdistance;
    float fogstart = 0.5f;

    culldistance = fogdist;

    //FOG_COLOR
    ::math::vector_4d fogcolor(skies->colorSet[FOG_COLOR], 1);
    glFogfv(GL_FOG_COLOR, fogcolor);
    //! \todo  retreive fogstart and fogend from lights.lit somehow
    glFogf(GL_FOG_END, fogdist);
    glFogf(GL_FOG_START, fogdist * fogstart);

    glEnable(GL_FOG);
  }
  else
  {
    glDisable(GL_FOG);
    culldistance = mapdrawdistance;
  }
}

void World::draw ( bool draw_terrain_height_contour
                 , bool mark_impassable_chunks
                 , bool draw_area_id_overlay
                 , bool dont_draw_cursor
                 , float inner_cursor_radius
                 , float outer_cursor_radius
                 , bool draw_wmo_doodads
                 , bool draw_fog
                 , bool draw_wmos
                 , bool draw_terrain
                 , bool draw_doodads
                 , bool draw_lines
                 , bool draw_hole_lines
                 , bool draw_water
                 , const QPointF& mouse_position
                 )
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  gluLookAt(camera.x(),camera.y(),camera.z(), lookat.x(),lookat.y(),lookat.z(), 0, 1, 0);

  frustum.retrieve();

  ///glDisable(GL_LIGHTING);
  ///glColor4f(1,1,1,1);

  bool had_sky (false);
  if (draw_wmos || mHasAGlobalWMO)
  {
    foreach (const wmo_instance_type& itr, mWMOInstances)
    {
      if ( itr.second->wmo->drawSkybox ( this
                                      , camera
                                      , itr.second->extents[0]
                                      , itr.second->extents[1]
                                      )
         )
      {
        had_sky = true;
        break;
      }
    }
  }

  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);

  int daytime = static_cast<int>(time) % 2880;
  outdoorLightStats = ol->getLightStats(daytime);
  skies->initSky(camera, daytime);

  if (!had_sky)
    had_sky = skies->drawSky(this, camera);

  // clearing the depth buffer only - color buffer is/has been overwritten anyway
  // unless there is no sky OR skybox
  GLbitfield clearmask = GL_DEPTH_BUFFER_BIT;
  if (!had_sky)   clearmask |= GL_COLOR_BUFFER_BIT;
  glClear(clearmask);

  glDisable(GL_TEXTURE_2D);

  outdoorLighting();
  outdoorLights(true);

  glFogi(GL_FOG_MODE, GL_LINEAR);
  setupFog (draw_fog);

  // Draw verylowres heightmap
  if (draw_fog && draw_terrain) {
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor3fv(skies->colorSet[FOG_COLOR]);
    //glColor3f(0,1,0);
    //glDisable(GL_FOG);
    const int lrr = 2;
    for (int i=cx-lrr; i<=cx+lrr; ++i) {
      for (int j=cz-lrr; j<=cz+lrr; ++j) {
        //! \todo  some annoying visual artifacts when the verylowres terrain overlaps
        // maptiles that are close (1-off) - figure out how to fix.
        // still less annoying than hoels in the horizon when only 2-off verylowres tiles are drawn
        if ( !(i==cx&&j==cz) && oktile(i,j) && lowrestiles[j][i])
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

  const bool enable_shaders
    (GLEW_ARB_vertex_program && GLEW_ARB_fragment_program);

  // if we're using shaders let's give it some specular
  if (enable_shaders) {
    ::math::vector_4d spec_color(0.1f,0.1f,0.1f,0.1f);
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


  // gosh darn alpha blended evil
  //! \note THIS SCOPE IS NEEDED FOR THE SETTINGS SAVER!
  {
    opengl::settings_saver saver;

    // height map w/ a zillion texture passes
    //! \todo  Do we need to push the matrix here?

    glPushMatrix();

    if( draw_terrain )
    {
      for( int j = 0; j < 64; ++j )
      {
        for( int i = 0; i < 64; ++i )
        {
          if( tileLoaded( j, i ) )
          {
            mTiles[j][i].tile->draw ( draw_terrain_height_contour
                                    , mark_impassable_chunks
                                    , draw_area_id_overlay
                                    , dont_draw_cursor
                                    , animtime
                                    , skies
                                    );
          }
        }
      }
    }

    glPopMatrix();

    // Selection circle
    if( IsSelection( eEntry_MapChunk )  )
    {
      //int poly = GetCurrentSelectedTriangle();

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

      //nameEntry * Selection = GetCurrentSelection();

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


      winX = mouse_position.x();
      winY = (float)viewport[3] - mouse_position.y();

      glReadPixels( winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
      gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

      _exact_terrain_selection_position = ::math::vector_3d (posX, posY, posZ);

      glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
      glDisable(GL_CULL_FACE);
      //glDepthMask(false);
      //glDisable(GL_DEPTH_TEST);

      if (!dont_draw_cursor)
      {
        QSettings settings;
        //! \todo This actually should be an enum. And should be passed into this method.
        const int cursor_type (settings.value ("cursor/type", 1).toInt());
        if(cursor_type == 1)
          renderDisk_convenient(posX, posY, posZ, outer_cursor_radius);
        else if(cursor_type == 2)
          renderSphere_convenient(posX, posY, posZ, outer_cursor_radius, 15);
      }

      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      //GlDepthMask(true);
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    }


    if (draw_lines)
    {
      glDisable(GL_COLOR_MATERIAL);
      glActiveTexture(GL_TEXTURE0);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      setupFog (draw_fog);
      for( int j = 0; j < 64; ++j )
      {
        for( int i = 0; i < 64; ++i )
        {
          if( tileLoaded( j, i ) )
          {
            mTiles[j][i].tile->drawLines (draw_hole_lines);
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

    if (enable_shaders) {
      ::math::vector_4d spec_color(0,0,0,1);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
      glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
    }

    // unbind hardware buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);




    glEnable(GL_CULL_FACE);

    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);

    // TEMP: for fucking around with lighting
    for(opengl::light light = GL_LIGHT0; light < GL_LIGHT0 + 8; ++light )
    {
      glLightf(light, GL_CONSTANT_ATTENUATION, l_const);
      glLightf(light, GL_LINEAR_ATTENUATION, l_linear);
      glLightf(light, GL_QUADRATIC_ATTENUATION, l_quadratic);
    }





    // M2s / models
    if( draw_doodads)
    {
      ModelManager::resetAnim();

      glEnable(GL_LIGHTING);  //! \todo  Is this needed? Or does this fuck something up?
      for( std::map<int, ModelInstance*>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it )
        it->second->draw (draw_fog);

      //drawModelList();
    }




    // WMOs / map objects
    if( draw_wmos || mHasAGlobalWMO )
      if (enable_shaders)
      {
        ::math::vector_4d spec_color( 1.0f, 1.0f, 1.0f, 1.0f );
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
        glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 10 );

        glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );

        for( std::map<int, WMOInstance *>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
          it->second->draw (draw_wmo_doodads, draw_fog, skies->hasSkies(), animtime, culldistance);

        spec_color = ::math::vector_4d( 0.0f, 0.0f, 0.0f, 1.0f );
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
        glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 0 );
      }
      else
        for( std::map<int, WMOInstance *>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
          it->second->draw (draw_wmo_doodads, draw_fog, skies->hasSkies(), animtime, culldistance);

    outdoorLights( true );
    setupFog (draw_fog);

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    glDisable(GL_CULL_FACE);

    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_LIGHTING);
  }

  setupFog (draw_fog);


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
  glDisable(GL_COLOR_MATERIAL);

  if(draw_water)
  {
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if( tileLoaded( j, i ) )
        {
          mTiles[j][i].tile->drawWater (animtime, skies);
        }
      }
    }
  }


  ex = camera.x() / TILESIZE;
  ez = camera.z() / TILESIZE;
}

enum stack_types
{
  MapObjName,
  DoodadName,
  MapTileName
};

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

void World::drawSelection ( bool draw_wmo_doodads
                          , bool draw_wmos
                          , bool draw_doodads
                          , bool draw_terrain
                          )
{
  glSelectBuffer ( sizeof (_selection_buffer) / sizeof (GLuint)
                 , _selection_buffer
                 );
  glRenderMode (GL_SELECT);

  glBindBuffer (GL_ARRAY_BUFFER, 0);

  gluLookAt ( camera.x(), camera.y(), camera.z()
            , lookat.x(), lookat.y(), lookat.z()
            , 0, 1, 0
            );

  frustum.retrieve();

  glClear (GL_DEPTH_BUFFER_BIT);

  glInitNames();

  if (draw_terrain)
  {
    ::opengl::scoped::name_pusher type (MapTileName);
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if( tileLoaded( j, i ) )
        {
          mTiles[j][i].tile->drawSelect (culldistance);
        }
      }
    }
  }

  if (draw_wmos)
  {
    ::opengl::scoped::name_pusher type (MapObjName);
    ::opengl::scoped::name_pusher dummy (0);
    for ( std::map<int, WMOInstance *>::iterator it (mWMOInstances.begin())
        ; it != mWMOInstances.end()
        ; ++it
        )
    {
      it->second->drawSelect (draw_wmo_doodads, animtime, culldistance);
    }
  }

  if (draw_doodads)
  {
    ModelManager::resetAnim();

    ::opengl::scoped::name_pusher type (DoodadName);
    ::opengl::scoped::name_pusher dummy (0);
    for ( std::map<int, ModelInstance*>::iterator it (mModelInstances.begin())
        ; it != mModelInstances.end()
        ; ++it
        )
    {
      it->second->draw_for_selection();
    }
  }

  GLuint minDist = 0xFFFFFFFF;
  GLNameEntry* minEntry = NULL;
  GLuint hits = glRenderMode( GL_RENDER );

  size_t offset = 0;

  //! \todo Isn't the closest one always the first? Iterating would be worthless then.
  while( hits-- > 0U )
  {
    GLNameEntry* entry = reinterpret_cast<GLNameEntry*>( &_selection_buffer[offset] );

    // We always push { MapObjName | DoodadName | MapTileName }, { 0, 0, MapTile }, { UID, UID, triangle }
    assert( entry->stackSize == 3 );

    LogDebug << "Hit: " << entry->stack.type << ", " << entry->stack.dummy << ", " << entry->stack.uniqueId << std::endl;

    if( entry->nearZ < minDist )
    {
      minDist = entry->nearZ;
      minEntry = entry;
    }

    offset += sizeof( GLNameEntry ) / sizeof( GLuint );
  }

  if( minEntry )
  {
    LogDebug << "minEntry: " << minEntry->stack.type << ", " << minEntry->stack.dummy << ", " << minEntry->stack.uniqueId << std::endl << std::endl;

    if( minEntry->stack.type == MapObjName || minEntry->stack.type == DoodadName )
    {
      mCurrentSelection = selection_names().findEntry( minEntry->stack.uniqueId );
      assert (mCurrentSelection);
    }
    else if( minEntry->stack.type == MapTileName )
    {
      mCurrentSelection = selection_names().findEntry( minEntry->stack.chunk );
      mCurrentSelectedTriangle = minEntry->stack.triangle;
    }
  }
}

void World::tick(float dt)
{
  enterTile(ex,ez);

  while (dt > 0.1f) {
    ModelManager::updateEmitters(0.1f);
    dt -= 0.1f;
  }
  ModelManager::updateEmitters(dt);
}

const unsigned int World::getAreaID() const
{
  const int mtx = camera.x() / TILESIZE;
  const int mtz = camera.z() / TILESIZE;
  const int mcx = fmod(camera.x(), TILESIZE) / CHUNKSIZE;
  const int mcz = fmod(camera.z(), TILESIZE) / CHUNKSIZE;

  if((mtx<cx-1) || (mtx>cx+1) || (mtz<cz-1) || (mtz>cz+1))
    return 0;

  MapTile* curTile = mTiles[mtz][mtx].tile;
  if(!curTile)
    return 0;

  MapChunk *curChunk = curTile->getChunk(mcx, mcz);
  if(!curChunk)
    return 0;

  return curChunk->areaID;
}

void World::clearHeight(int x, int z)
{
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile;
      curTile = mTiles[z][x].tile;
      if(curTile == 0) continue;
      setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      if(curChunk == 0) continue;

      curChunk->vmin.y (9999999.0f);
      curChunk->vmax.y (-9999999.0f);
      curChunk->Changed=true;

      for(int i=0; i < mapbufsize; ++i)
      {
        curChunk->mVertices[i].y (0.0f);

        curChunk->vmin.y (std::min(curChunk->vmin.y(),curChunk-> mVertices[i].y()));
        curChunk->vmax.y (std::max(curChunk->vmax.y(), curChunk->mVertices[i].y()));
      }

      glBindBuffer(GL_ARRAY_BUFFER, curChunk->vertices);
      glBufferData(GL_ARRAY_BUFFER, sizeof(curChunk->mVertices), curChunk->mVertices, GL_STATIC_DRAW);
    }
  }

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile;
      curTile = mTiles[z][x].tile;
      if(curTile == 0) continue;
      setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->recalcNorms();
    }
  }
}

void World::clearAllModelsOnADT(int x,int z)
{
  // get the adt
  MapTile *curTile;
  curTile = mTiles[z][x].tile;
  if(curTile == 0) return;
  curTile->clearAllModels();
}

void World::setAreaID(int id, int x,int z)
{
  // set the Area ID on a tile x,z on all chunks
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      setAreaID(id, x, z, j, i);
    }
  }
}

void World::setAreaID(int id, int x, int z , int _cx, int _cz)
{

  // set the Area ID on a tile x,z on the chunk cx,cz
  MapTile *curTile;
  curTile = mTiles[z][x].tile;
  if(curTile == 0) return;
  setChanged(z,x);
  MapChunk *curChunk = curTile->getChunk(_cx, _cz);

  if(curChunk == 0) return;

  curChunk->areaID = id;
}

void World::drawTileMode ( bool draw_lines
                         , float ratio
                         , float zoom
                         )
{
  const QRectF drawing_rect ( camera.x() / CHUNKSIZE - 2.0f * ratio / zoom
                            , camera.z() / CHUNKSIZE - 2.0f / zoom
                            , 4.0f * ratio / zoom
                            , 4.0f / zoom
                            );

  ex = camera.x() / TILESIZE;
  ez = camera.z() / TILESIZE;

  glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glEnable (GL_BLEND);

  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnableClientState (GL_COLOR_ARRAY);
  glDisableClientState (GL_NORMAL_ARRAY);
  glDisableClientState (GL_TEXTURE_COORD_ARRAY);
  glDisable (GL_CULL_FACE);
  glDepthMask (GL_FALSE);

  glPushMatrix();
  glScalef (zoom, zoom, 1.0f);

  glPushMatrix();
  glTranslatef (-camera.x() / CHUNKSIZE, -camera.z() / CHUNKSIZE, 0.0f);

  //! \todo Only iterate over those intersecting?
  for (size_t j (0); j < 64; ++j)
  {
    for (size_t i (0); i < 64; ++i)
    {
      if (tileLoaded (j, i))
      {
        const MapTile* tile (mTiles[j][i].tile);
        const QRectF map_rect ( tile->xbase / CHUNKSIZE
                              , tile->zbase / CHUNKSIZE
                              , TILESIZE / CHUNKSIZE
                              , TILESIZE / CHUNKSIZE
                              );

        if (drawing_rect.intersects (map_rect))
        {
          tile->drawTextures ( drawing_rect.intersected (map_rect)
                                           .translated (-map_rect.topLeft())
                             , animtime
                             );
        }
      }
    }
  }

  glPopMatrix();

  if (draw_lines)
  {
    glTranslatef(fmod(-camera.x()/CHUNKSIZE,16), fmod(-camera.z()/CHUNKSIZE,16),0);
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

  glDisableClientState (GL_COLOR_ARRAY);

  glEnableClientState (GL_NORMAL_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
}

bool World::GetVertex(float x,float z, ::math::vector_3d *V)
{
  const int newX = x / TILESIZE;
  const int newZ = z / TILESIZE;

  if( !tileLoaded( newZ, newX ) )
  {
    return false;
  }

  return mTiles[newZ][newX].tile->GetVertex(x, z, V);
}

void World::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( mTiles[j][i].tile->getChunk(ty,tx)->changeTerrain(x,z,change,radius,BrushType) )
              setChanged( j, i );
          }
        }
      }
    }
  }

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->recalcNorms();
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
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( mTiles[j][i].tile->getChunk(ty,tx)->flattenTerrain(x,z,h,remain,radius,BrushType) )
              setChanged(j,i);
          }
        }
      }
    }
  }

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->recalcNorms();
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
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( mTiles[j][i].tile->getChunk(ty,tx)->blurTerrain(x, z, remain, radius, BrushType) )
              setChanged(j,i);
          }
        }
      }
    }
  }

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}

bool World::paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::blp_texture* texture)
{
  //const int newX = (int)(x / TILESIZE);
  //const int newZ = (int)(z / TILESIZE);

  //Log << "Painting Textures at " << x << " and " << z;
  bool succ = false;

  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( mTiles[j][i].tile->getChunk( ty, tx )->paintTexture( x, z, Brush, strength, pressure, texture ) )
            {
              succ |= true;
              setChanged( j, i );
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
  setChanged(x,z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;
  Log << "Erasing Textures at " << x << " and " << z;
  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              chunk->eraseTextures();
            }
          }
        }
      }
    }
  }
}

void World::overwriteTextureAtCurrentChunk(float x, float z, noggit::blp_texture* oldTexture, noggit::blp_texture* newTexture)
{
  setChanged(x,z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;
  Log << "Switching Textures at " << x << " and " << z;
  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
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

void World::addHole( float x, float z )
{
  setChanged(x, z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;

  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
              int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
              chunk->addHole( k, l );
            }
          }
        }
      }
    }
  }
}

void World::removeHole( float x, float z )
{
  setChanged(x, z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;

  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
              int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
              chunk->removeHole( k, l );
            }
          }
        }
      }
    }
  }
}

void World::saveMap()
{
  assert (!"This is currently broken.");

  //! \todo  Output as BLP.
  unsigned char image[256*256*3];
  MapTile *ATile;
  FILE *fid;
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glReadBuffer(GL_BACK);

  glEnableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  for(int y=0;y<64;y++)
  {
    for(int x=0;x<64;x++)
    {
      if( !( mTiles[y][x].flags & 1 ) )
      {
        continue;
      }

      ATile=loadTile(x,y);
      glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

      glPushMatrix();
      glScalef(0.08333333f,0.08333333f,1.0f);

      //glTranslatef(-camera.x()/CHUNKSIZE,-camera.z()/CHUNKSIZE,0);
      glTranslatef( x * -16.0f - 8.0f, y * -16.0f - 8.0f, 0.0f );

      ATile->drawTextures (QRect (0, 0, 16, 16), animtime);
      glPopMatrix();

  //! \todo Fix these two lines. THEY ARE VITAL!
//    glReadPixels (video.xres()/2-128, video.yres()/2-128, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, image);
//      swapBuffers();

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
  std::map<int, ModelInstance*>::iterator it = mModelInstances.find( pUniqueID );
  setChanged( it->second->pos.x(), it->second->pos.z() );
  delete it->second;
  mModelInstances.erase( it );
  ResetSelection();
}

void World::deleteWMOInstance( int pUniqueID )
{
  std::map<int, WMOInstance *>::iterator it = mWMOInstances.find( pUniqueID );
  setChanged( it->second->pos.x(), it->second->pos.z() );
  mWMOInstances.erase( it );
  ResetSelection();
}

void World::addModel ( const nameEntry& entry
                     , ::math::vector_3d newPos
                     , bool size_randomization
                     , bool position_randomization
                     , bool rotation_randomization
                     )
{
  if( entry.type == eEntry_Model )
    addM2 ( entry.data.model->model
          , newPos
          , size_randomization
          , position_randomization
          , rotation_randomization
          );
  else if( entry.type == eEntry_WMO )
    addWMO( entry.data.wmo->wmo, newPos );
}

void World::addM2 ( Model* model
                  , ::math::vector_3d newPos
                  , bool size_randomization
                  , bool position_randomization
                  , bool rotation_randomization
                  )
{
  //! \todo This _will_ fuck up as you may hit a WMO uid.
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

  ModelInstance *newModelis = new ModelInstance(this, model);
  newModelis->d1 = lMaxUID;
  newModelis->pos = newPos;
  newModelis->sc = 1;
  if (rotation_randomization)
  {
    newModelis->dir.y (newModelis->dir.y() + ::math::random::floating_point (0.0f, 360.0f));
  }

  if (position_randomization)
  {
    newModelis->pos.x ( newModelis->pos.x()
                     + ::math::random::floating_point (-2.0f, 2.0f)
                     );
    newModelis->pos.z ( newModelis->pos.z()
                     + ::math::random::floating_point (-2.0f, 2.0f)
                     );
  }

  if (size_randomization)
  {
    newModelis->sc *= ::math::random::floating_point (0.9f, 1.1f);
  }

  mModelInstances.insert( std::pair<int,ModelInstance*>( lMaxUID, newModelis ));
  setChanged(newPos.x(), newPos.z());
}

void World::addWMO( WMO *wmo, ::math::vector_3d newPos )
{
  const int lMaxUID = std::max( ( mModelInstances.empty() ? 0 : mModelInstances.rbegin()->first + 1 ),
                           ( mWMOInstances.empty() ? 0 : mWMOInstances.rbegin()->first + 1 ) );

  WMOInstance *newWMOis = new WMOInstance(this, wmo);
  newWMOis->pos = newPos;
  newWMOis->mUniqueID = lMaxUID;
  mWMOInstances.insert( std::pair<int,WMOInstance *>( lMaxUID, newWMOis ));
  setChanged(newPos.x(),newPos.z());
}

static int tile_below_camera (const float& position)
{
  return ::math::bounded_nearest<int> ((position - (TILESIZE / 2)) / TILESIZE);
}

void World::setChanged(float x, float z)
{
  // change the changed flag of the map tile
  int row (tile_below_camera (x));
  int column (tile_below_camera (z));
  if( row >= 0 && row <= 64 && column >= 0 && column <= 64 )
    if( mTiles[column][row].tile )
      mTiles[column][row].tile->changed = true;
}

void World::setChanged(int x, int z)
{
  // change the changed flag of the map tile
  if( mTiles[x][z].tile )
    mTiles[x][z].tile->changed = true;
}

void World::unsetChanged(int x, int z)
{
  // change the changed flag of the map tile
  if( mTiles[x][z].tile )
    mTiles[x][z].tile->changed = false;
}

bool World::getChanged(int x, int z) const
{
  if(mTiles[x][z].tile)
    return mTiles[x][z].tile->changed;
  else return false;
}

void World::setFlag( bool to, float x, float z)
{
  // set the inpass flag to selected chunk
  setChanged(x, z);
  const int newX = x / TILESIZE;
  const int newZ = z / TILESIZE;

  for( int j = newZ - 1; j < newZ + 1; ++j )
  {
    for( int i = newX - 1; i < newX + 1; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ++ty )
        {
          for( int tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              chunk->setFlag(to, FLAG_IMPASS);
            }
          }
        }
      }
    }
  }
}

const unsigned int World::getMapID() const
{
  return mMapId;
}

const float World::getAnimtime() const
{
    return animtime;
}

void World::setAnimtime(float newTime)
{
    animtime = newTime;
}

const float World::getTime() const
{
    return time;
}

void World::setTime(float newTime)
{
    time = newTime;
}

const float World::getFogdistance() const
{
    return fogdistance;
}

void World::setFogdistance(float distance)
{
    fogdistance = distance;
}

void World::moveHeight(int x, int z)
{
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile;
      curTile = mTiles[z][x].tile;
      if(curTile == 0) continue;
      setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      if(curChunk == 0) continue;

      curChunk->vmin.y (9999999.0f);
      curChunk->vmax.y (-9999999.0f);
      curChunk->Changed = true;

      float heightDelta = 0.0f;
      nameEntry *selection = GetCurrentSelection();

      if(selection)
        if(selection->type == eEntry_MapChunk)
        {
          // chunk selected
          heightDelta = camera.y() - selection->data.mapchunk->py;
        }

      if( heightDelta * heightDelta <= 0.1f ) continue;

      for(int i=0; i < mapbufsize; ++i)
      {
        curChunk->mVertices[i].y (curChunk->mVertices[i].y() + heightDelta);

        curChunk->vmin.y (std::min (curChunk->vmin.y(), curChunk-> mVertices[i].y()));
        curChunk->vmax.y (std::max (curChunk->vmax.y(), curChunk->mVertices[i].y()));
      }

      glBindBuffer(GL_ARRAY_BUFFER, curChunk->vertices);
      glBufferData(GL_ARRAY_BUFFER, sizeof(curChunk->mVertices), curChunk->mVertices, GL_STATIC_DRAW);
    }
  }

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile = mTiles[z][x].tile;
      if(curTile == 0) continue;
      setChanged(z,x);
      curTile->getChunk(j, i)->recalcNorms();
    }
  }

}

void World::setBaseTexture( int x, int z, noggit::blp_texture* texture )
{
  MapTile *curTile;
  curTile = mTiles[z][x].tile;
  if(curTile == 0) return;

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->eraseTextures();
      if (texture)
      {
        curChunk->addTexture( texture );
        texture->addReference();
      }
    }
  }
}


void World::saveWDT()
{
   // int lCurrentPosition = 0;
    //sExtendableArray lWDTFile = sExtendableArray();
   // lWDTFile.Extend( 8 + 0x4 );
   // SetChunkHeader( lWDTFile, lCurrentPosition, 'MPHD', 4 );

   // noggit::mpq::file f( "test.WDT" );
   // f.setBuffer( lWDTFile.GetPointer<uint8_t>(), lWDTFile.mSize );
   // f.SaveFile();
   // f.close();
}
