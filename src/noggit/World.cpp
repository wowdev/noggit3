// This file is part of Noggit3, licensed under GNU General Public License (version 3).

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
#include <opengl/context.h>
#include <opengl/matrix.h>
#include <opengl/scoped.h>
#include <opengl/settings_saver.h>

#include <noggit/application.h>
#include <noggit/blp_texture.h>
#include <noggit/Brush.h>
#include <noggit/DBC.h>
#include <noggit/Frustum.h> // Frustum
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/ModelManager.h>
#include <noggit/TextureManager.h>
#include <noggit/UITexturingGUI.h>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/MapTile.h>
#include <noggit/mpq/file.h>

namespace
{
  std::size_t const sphere_segments (15);
  void draw_sphere_point (int i, int j, float radius)
  {
    static float const drho (::math::constants::pi() / sphere_segments);
    static float const dtheta (2.0f * ::math::constants::pi() / sphere_segments);

    float const rho (i * drho);
    float const theta (j * dtheta);
    gl.vertex3f ( std::cos (theta) * std::sin (rho) * radius
               , std::sin (theta) * std::sin (rho) * radius
               , std::cos (rho) * radius
               );
  }
  void draw_sphere (float radius)
  {
    for (int i = 1; i < sphere_segments; i++)
    {
      gl.begin (GL_LINE_LOOP);
      for (int j = 0; j < sphere_segments; j++)
      {
        draw_sphere_point (i, j, radius);
      }
      gl.end();
    }

    for (int j = 0; j < sphere_segments; j++)
    {
      gl.begin(GL_LINE_STRIP);
      for (int i = 0; i <= sphere_segments; i++)
      {
        draw_sphere_point (i, j, radius);
      }
      gl.end();
    }
  }
  void renderSphere_convenient(::math::vector_3d const& position, float radius)
  {
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting;

    //! \todo This should be passed in!
    QSettings settings;
    gl.color4f ( settings.value ("cursor/red", 1.0f).toFloat()
              , settings.value ("cursor/green", 1.0f).toFloat()
              , settings.value ("cursor/blue", 1.0f).toFloat()
              , settings.value ("cursor/alpha", 1.0f).toFloat()
              );

    opengl::scoped::matrix_pusher matrix;

    gl.translatef (position.x(), position.y(), position.z());

    draw_sphere (0.3f);
    draw_sphere (radius);
  }

  void draw_disk_point (float radius, float arc)
  {
    gl.vertex3f (radius * std::sin (arc), radius * std::cos (arc), 0.0f);
  }
  void draw_disk (float radius)
  {
    int const slices (std::max (15.0f, radius * 1.5f));
    static float const max (2.0f * ::math::constants::pi());
    float const stride (max / slices);

    gl.begin (GL_LINE_LOOP);
    for (float arc (0.0f); arc < max; arc += stride)
    {
      draw_disk_point (radius, arc);
    }
    gl.end();

    gl.begin (GL_LINE_LOOP);
    for (float arc (0.0f); arc < max; arc += stride)
    {
      draw_disk_point (radius + 1.0f, arc);
    }
    gl.end();

    for (float arc (0.0f); arc < max; arc += stride)
    {
      gl.begin (GL_LINES);
      draw_disk_point (radius, arc);
      draw_disk_point (radius + 1.0f, arc);
      gl.end();
     }
  }

  void renderDisk_convenient(::math::vector_3d const& position, float radius)
  {
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting;

    {
      opengl::scoped::matrix_pusher matrix;
      opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth_test;
      gl.colorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
      opengl::scoped::bool_setter<GL_COLOR_MATERIAL, GL_TRUE> color_material;

      gl.translatef (position.x(), position.y(), position.z());
      gl.rotatef (90.0f, 1.0f, 0.0f, 0.0f);

      //! \todo This should be passed in!
      QSettings settings;
      gl.color4f ( settings.value ("cursor/red", 1.0f).toFloat()
                , settings.value ("cursor/green", 1.0f).toFloat()
                , settings.value ("cursor/blue", 1.0f).toFloat()
                , settings.value ("cursor/alpha", 1.0f).toFloat()
                );

      draw_disk (radius);
    }

    {
      opengl::scoped::matrix_pusher matrix;

      gl.translatef (position.x(), position.y(), position.z());

      draw_sphere (0.3f);
    }
  }
}

MapTileEntry::MapTileEntry() : flags( 0 ), tile( nullptr ) {}

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
  : time( 1450 )
  , camera( ::math::vector_3d( 0.0f, 0.0f, 0.0f ) )
  , lookat( ::math::vector_3d( 0.0f, 0.0f, 0.0f ) )
  , outdoorLightStats( OutdoorLightStats() )
  , mBigAlpha( false )
  , _initialized_display (false)
  , detailtexcoords( 0 )
  , alphatexcoords( 0 )
  , mMapId( 0xFFFFFFFF )
  , basename( name )
  , _selection_names()
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
      lowrestiles[j][i] = nullptr;
      _tile_got_modified[j][i] = false;
    }
  }

  const QString filename
    ( QString ("World\\Maps\\%1\\%1.wdt")
      .arg (QString::fromStdString (basename))
    );

  noggit::mpq::file theFile (filename);
  uint32_t fourcc;

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

  //! \note This indicates a global WMO, which we don't want to edit, as Noggit
  //        is an ADT based map editor.
  assert (!(flags & 1));

  mBigAlpha = flags & 4;

  // - MAIN ----------------------------------------------

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MAIN' );

  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      theFile.read( &mTiles[j][i].flags, 4 );
      theFile.seekRelative( 4 );
    }
  }

  // -----------------------------------------------------

  theFile.close();

  initMinimap();
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

        lowrestiles[y][x].reset (new opengl::call_list);
        lowrestiles[y][x]->start_recording();

        //! \todo Make a strip out of this.
        gl.begin( GL_TRIANGLES );
        for (size_t j (0); j < 16; ++j )
        {
          for (size_t i (0); i < 16; ++i )
          {
            gl.vertex3fv (vertices_17[j][i]);
            gl.vertex3fv (vertices_16[j][i]);
            gl.vertex3fv (vertices_17[j][i + 1]);
            gl.vertex3fv (vertices_17[j][i + 1]);
            gl.vertex3fv (vertices_16[j][i]);
            gl.vertex3fv (vertices_17[j + 1][i + 1]);
            gl.vertex3fv (vertices_17[j + 1][i + 1]);
            gl.vertex3fv (vertices_16[j][i]);
            gl.vertex3fv (vertices_17[j + 1][i]);
            gl.vertex3fv (vertices_17[j + 1][i]);
            gl.vertex3fv (vertices_16[j][i]);
            gl.vertex3fv (vertices_17[j][i]);
          }
        }
        gl.end();

        lowrestiles[y][x]->end_recording();

        //! \todo There also is MAHO giving holes into this heightmap.
      }
    }
  }
}

namespace
{
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

      gl.genBuffers(1, pDetailTexCoords);
      gl.bindBuffer(GL_ARRAY_BUFFER, *pDetailTexCoords);
      gl.bufferData(GL_ARRAY_BUFFER, sizeof(temp), temp, GL_STATIC_DRAW);

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

      gl.genBuffers(1, pAlphaTexCoords);
      gl.bindBuffer(GL_ARRAY_BUFFER, *pAlphaTexCoords);
      gl.bufferData(GL_ARRAY_BUFFER, sizeof(temp), temp, GL_STATIC_DRAW);

      gl.bindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }
}

void World::initDisplay()
{
  initGlobalVBOs( &detailtexcoords, &alphatexcoords );

  skies.reset (new Skies (mMapId));

  ol.reset (new OutdoorLighting ("World\\dnc.db"));

  initLowresTerrain();
}

World::~World()
{
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

void World::load_tiles_around ( const size_t& x
                              , const size_t& z
                              , const size_t& distance
                              )
{
  for ( size_t i (std::max<size_t> (z - distance, 0))
      ; i < std::min<size_t> (z + distance, 64)
      ; ++i
      )
  {
    for ( size_t j (std::max<size_t> (x - distance, 0))
        ; j < std::min<size_t> (x + distance, 64)
        ; ++j
        )
    {
      if (hasTile (i, j))
      {
        loadTile (i, j);
      }
    }
  }
}

void World::reloadTile (int x, int z)
{
  if (tileLoaded (z, x))
  {
    mTiles[z][x].tile.reset();

    loadTile (z, x);
  }
}

void World::saveTile(int x, int z) const
{
  if( tileLoaded( z, x ) )
  {
    mTiles[z][x].tile->saveTile ( mModelInstances.begin()
                                , mModelInstances.end()
                                , mWMOInstances.begin()
                                , mWMOInstances.end()
                                );
  }
}

void World::saveTileCata(int x, int z) const
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

inline bool World::tileLoaded(int z, int x) const
{
  return hasTile( z, x ) && mTiles[z][x].tile;
}

MapTile* World::loadTile(int z, int x)
{
  if( !hasTile( z, x ) )
  {
    return nullptr;
  }

  if( tileLoaded( z, x ) )
  {
    return mTiles[z][x].tile.get();
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
    return nullptr;
  }

  mTiles[z][x].tile.reset (new MapTile( this, x, z, filename.toStdString(), mBigAlpha ));
  return mTiles[z][x].tile.get();
}

void World::outdoorLighting()
{
  ::math::vector_4d black(0,0,0,0);
  ::math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
  gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  float di = outdoorLightStats.dayIntensity;
  //float ni = outdoorLightStats.nightIntensity;

  ::math::vector_3d dd = outdoorLightStats.dayDir;
  // HACK: let's just keep the light source in place for now
  //::math::vector_4d pos(-1, 1, -1, 0);
  ::math::vector_4d pos(-dd.x(), -dd.z(), dd.y(), 0.0f);
  ::math::vector_4d col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1.0f);
  gl.lightfv(GL_LIGHT0, GL_AMBIENT, black);
  gl.lightfv(GL_LIGHT0, GL_DIFFUSE, col);
  gl.lightfv(GL_LIGHT0, GL_POSITION, pos);

  /*
  dd = outdoorLightStats.nightDir;
  pos(-dd.x(), -dd.z(), dd.y(), 0.0f);
  col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1.0f);
  gl.lightfv(GL_LIGHT1, GL_AMBIENT, black);
  gl.lightfv(GL_LIGHT1, GL_DIFFUSE, col);
  gl.lightfv(GL_LIGHT1, GL_POSITION, pos);*/
}

void World::outdoorLights(bool on)
{
  float di = outdoorLightStats.dayIntensity;
  float ni = outdoorLightStats.nightIntensity;

  if (on) {
    ::math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
    gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    if (di>0) {
      gl.enable(GL_LIGHT0);
    } else {
      gl.disable(GL_LIGHT0);
    }
    if (ni>0) {
      gl.enable(GL_LIGHT1);
    } else {
      gl.disable(GL_LIGHT1);
    }
  } else {
    ::math::vector_4d ambient(0, 0, 0, 1);
    gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    gl.disable(GL_LIGHT0);
    gl.disable(GL_LIGHT1);
  }
}

void World::setupFog (bool draw_fog, const float& fog_distance)
{
  if (draw_fog)
  {
    const ::math::vector_4d fogcolor (skies->colorSet[FOG_COLOR], 1);
    gl.fogfv (GL_FOG_COLOR, fogcolor);
    //! \todo  retreive fogstart and fogend from lights.lit somehow
    gl.fogf (GL_FOG_END, fog_distance);
    gl.fogf (GL_FOG_START, fog_distance * 0.5f);

    gl.enable(GL_FOG);
  }
  else
  {
    gl.disable(GL_FOG);
  }
}

void World::draw ( size_t flags
                 , float inner_cursor_radius
                 , float outer_cursor_radius
                 , const QPointF& mouse_position
                 , const float& fog_distance
                 , const boost::optional<selection_type>& selected_item
                 )
{
  if (!_initialized_display)
  {
    initDisplay();
    _initialized_display = true;
  }

  const int cx (camera.x() / TILESIZE);
  const int cz (camera.z() / TILESIZE);

  gl.bindBuffer(GL_ARRAY_BUFFER, 0);

  opengl::matrix::look_at (camera, lookat, {0.0f, 1.0f, 0.0f});

  const Frustum frustum;

  ///gl.disable(GL_LIGHTING);
  ///gl.color4f(1,1,1,1);

  bool had_sky (false);
  if (flags & DRAWWMO)
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

  gl.enable(GL_CULL_FACE);
  gl.disable(GL_BLEND);
  gl.disable(GL_TEXTURE_2D);
  gl.disable(GL_DEPTH_TEST);
  gl.disable(GL_FOG);

  int daytime = static_cast<int>(time) % 2880;
  outdoorLightStats = ol->getLightStats(daytime);
  skies->initSky(camera, daytime);

  if (!had_sky)
    had_sky = skies->drawSky(this, camera);

  // clearing the depth buffer only - color buffer is/has been overwritten anyway
  // unless there is no sky OR skybox
  GLbitfield clearmask = GL_DEPTH_BUFFER_BIT;
  if (!had_sky)   clearmask |= GL_COLOR_BUFFER_BIT;
  gl.clear(clearmask);

  gl.disable(GL_TEXTURE_2D);

  outdoorLighting();
  outdoorLights(true);

  gl.fogi(GL_FOG_MODE, GL_LINEAR);
  setupFog (flags & FOG, fog_distance);

  // Draw verylowres heightmap
  if ((flags & FOG) && (flags & TERRAIN)) {
    gl.enable(GL_CULL_FACE);
    gl.disable(GL_DEPTH_TEST);
    gl.disable(GL_LIGHTING);
    gl.color3fv(skies->colorSet[FOG_COLOR]);
    //gl.color3f(0,1,0);
    //gl.disable(GL_FOG);
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
    //gl.enable(GL_FOG);
  }

  // Draw height map
  gl.enableClientState(GL_VERTEX_ARRAY);
  gl.enableClientState(GL_NORMAL_ARRAY);

  gl.enable(GL_DEPTH_TEST);
  gl.depthFunc(GL_LEQUAL); // less z-fighting artifacts this way, I think
  gl.enable(GL_LIGHTING);

  gl.enable(GL_COLOR_MATERIAL);
  //gl.colorMaterial(GL_FRONT, GL_DIFFUSE);
  gl.colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  gl.color4f(1,1,1,1);

  ::math::vector_4d spec_color(0.1f,0.1f,0.1f,0.1f);
  gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
  gl.materiali(GL_FRONT_AND_BACK, GL_SHININESS, 5);

  gl.lightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

  gl.enable(GL_BLEND);
  gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gl.clientActiveTexture(GL_TEXTURE0);
  gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
  gl.bindBuffer(GL_ARRAY_BUFFER, detailtexcoords);
  gl.texCoordPointer(2, GL_FLOAT, 0, 0);

  gl.clientActiveTexture(GL_TEXTURE1);
  gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
  gl.bindBuffer(GL_ARRAY_BUFFER, alphatexcoords);
  gl.texCoordPointer(2, GL_FLOAT, 0, 0);

  gl.clientActiveTexture(GL_TEXTURE0);


  // gosh darn alpha blended evil
  //! \note THIS SCOPE IS NEEDED FOR THE SETTINGS SAVER!
  {
    opengl::settings_saver saver;

    // height map w/ a zillion texture passes
    //! \todo  Do we need to push the matrix here?

    {
      opengl::scoped::matrix_pusher const matrix_pusher;

      if( flags & TERRAIN )
      {
        for( int j = 0; j < 64; ++j )
        {
          for( int i = 0; i < 64; ++i )
          {
            if( tileLoaded( j, i ) )
            {
              mTiles[j][i].tile->draw ( flags & HEIGHTCONTOUR
                                      , flags & MARKIMPASSABLE
                                      , flags & AREAID
                                      , flags & NOCURSOR
                                      , skies.get()
                                      , mapdrawdistance
                                      , frustum
                                      , camera
                                      , selected_item
                                      );
            }
          }
        }
      }
    }

    // Selection circle
    if (selected_item && noggit::selection::is_chunk (*selected_item))
    {
      gl.polygonMode( GL_FRONT_AND_BACK, GL_LINE );

      GLint viewport[4];
      gl.getIntegerv (GL_VIEWPORT, viewport);

      float const win_x (mouse_position.x());
      float const win_y (static_cast<float> (viewport[3]) - mouse_position.y());
      float win_z;
      gl.readPixels (win_x, win_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);

      ::math::vector_4d const normalized_device_coords
        ( 2.0f * (win_x - static_cast<float> (viewport[0])) / static_cast<float> (viewport[2]) - 1.0f
        , 2.0f * (win_y - static_cast<float> (viewport[1])) / static_cast<float> (viewport[3]) - 1.0f
        , 2.0f * win_z - 1.0f
        , 1.0f
        );

      _exact_terrain_selection_position = ( ( opengl::matrix::model_view()
                                            * opengl::matrix::projection()
                                            ).inverted().transposed()
                                          * normalized_device_coords
                                          ).xyz_normalized_by_w();

      gl.color4f( 1.0f, 1.0f, 1.0f, 1.0f );
      gl.disable(GL_CULL_FACE);
      //gl.depthMask(false);
      //gl.disable(GL_DEPTH_TEST);

      if (!(flags & NOCURSOR))
      {
        QSettings settings;
        //! \todo This actually should be an enum. And should be passed into this method.
        const int cursor_type (settings.value ("cursor/type", 1).toInt());
        if(cursor_type == 1)
          renderDisk_convenient ( _exact_terrain_selection_position
                                , outer_cursor_radius
                                );
        else if(cursor_type == 2)
          renderSphere_convenient ( _exact_terrain_selection_position
                                  , outer_cursor_radius
                                  );
      }

      gl.enable(GL_CULL_FACE);
      gl.enable(GL_DEPTH_TEST);
      //GlDepthMask(true);
      gl.polygonMode( GL_FRONT_AND_BACK, GL_FILL );

    }


    if (flags & LINES)
    {
      gl.disable(GL_COLOR_MATERIAL);
      gl.activeTexture(GL_TEXTURE0);
      gl.disable(GL_TEXTURE_2D);
      gl.activeTexture(GL_TEXTURE1);
      gl.disable(GL_TEXTURE_2D);
      gl.enable(GL_BLEND);
      gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      setupFog (flags & FOG, fog_distance);
      for( int j = 0; j < 64; ++j )
      {
        for( int i = 0; i < 64; ++i )
        {
          if( tileLoaded( j, i ) )
          {
            mTiles[j][i].tile->drawLines ( flags & HOLELINES
                                         , mapdrawdistance
                                         , frustum
                                         , camera
                                         );
           // mTiles[j][i].tile->drawMFBO();
          }
        }
      }
    }

    gl.activeTexture(GL_TEXTURE1);
    gl.disable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE0);
    gl.enable(GL_TEXTURE_2D);

    gl.color4f(1,1,1,1);
    gl.enable(GL_BLEND);

    ::math::vector_4d spec_color(0,0,0,1);
    gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
    gl.materiali(GL_FRONT_AND_BACK, GL_SHININESS, 0);

    // unbind hardware buffers
    gl.bindBuffer(GL_ARRAY_BUFFER, 0);

    gl.enable(GL_CULL_FACE);

    gl.disable(GL_BLEND);
    gl.disable(GL_ALPHA_TEST);

    // TEMP: for fucking around with lighting
    for(opengl::light light = GL_LIGHT0; light < GL_LIGHT0 + 8; ++light )
    {
      const float l_const( 0.0f );
      const float l_linear( 0.7f );
      const float l_quadratic( 0.03f );
      gl.lightf(light, GL_CONSTANT_ATTENUATION, l_const);
      gl.lightf(light, GL_LINEAR_ATTENUATION, l_linear);
      gl.lightf(light, GL_QUADRATIC_ATTENUATION, l_quadratic);
    }

    // M2s / models
    if(flags & DOODADS)
    {
      noggit::app().model_manager().resetAnim();

      gl.enable(GL_LIGHTING);  //! \todo  Is this needed? Or does this fuck something up?
      for( std::map<int, ModelInstance*>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it )
      {
        if (it->second->is_visible (mapdrawdistance, frustum, camera))
        {
          it->second->draw (flags & FOG, selected_item);
        }
      }

      //drawModelList();
    }




    // WMOs / map objects
    if (flags & DRAWWMO)
    {
      ::math::vector_4d spec_color( 1.0f, 1.0f, 1.0f, 1.0f );
      gl.materialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
      gl.materiali( GL_FRONT_AND_BACK, GL_SHININESS, 10 );

      gl.lightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );

      for( std::map<int, WMOInstance *>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
        it->second->draw ( flags & WMODOODAS
                         , flags & FOG
                         , skies->hasSkies()
                         , (flags & FOG) ? fog_distance : mapdrawdistance
                         , fog_distance
                         , frustum
                         , camera
                         , selected_item
                         );

      spec_color = ::math::vector_4d( 0.0f, 0.0f, 0.0f, 1.0f );
      gl.materialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
      gl.materiali( GL_FRONT_AND_BACK, GL_SHININESS, 0 );
    }

    outdoorLights( true );
    setupFog (flags & FOG, fog_distance);

    gl.color4f( 1.0f, 1.0f, 1.0f, 1.0f );
    gl.disable(GL_CULL_FACE);

    gl.disable(GL_BLEND);
    gl.disable(GL_ALPHA_TEST);
    gl.enable(GL_LIGHTING);
  }

  setupFog (flags & FOG, fog_distance);


  gl.color4f(1,1,1,1);
  gl.enable(GL_BLEND);

  //gl.color4f(1,1,1,1);
  gl.disable(GL_COLOR_MATERIAL);

  if(flags & WATER)
  {
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if( tileLoaded( j, i ) )
        {
          mTiles[j][i].tile->drawWater (skies.get());
        }
      }
    }
  }
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

boost::optional<selection_type> World::drawSelection (size_t flags)
{
  static GLuint selection_buffer[8192];

  gl.selectBuffer ( sizeof (selection_buffer) / sizeof (GLuint)
                 , selection_buffer
                 );
  gl.renderMode (GL_SELECT);

  gl.bindBuffer (GL_ARRAY_BUFFER, 0);

  opengl::matrix::look_at (camera, lookat, {0.0f, 1.0f, 0.0f});

  const Frustum frustum;

  gl.clear (GL_DEPTH_BUFFER_BIT);

  gl.initNames();

  if (flags & TERRAIN)
  {
    ::opengl::scoped::name_pusher type (MapTileName);
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if( tileLoaded( j, i ) )
        {
          mTiles[j][i].tile->drawSelect ( mapdrawdistance
                                        , frustum
                                        , camera
                                        );
        }
      }
    }
  }

  if (flags & DRAWWMO)
  {
    ::opengl::scoped::name_pusher type (MapObjName);
    ::opengl::scoped::name_pusher dummy (0);
    for ( std::map<int, WMOInstance *>::iterator it (mWMOInstances.begin())
        ; it != mWMOInstances.end()
        ; ++it
        )
    {
      it->second->drawSelect ( flags & WMODOODAS
                             , mapdrawdistance
                             , frustum
                             , camera
                             );
    }
  }

  if (flags & DOODADS)
  {
    noggit::app().model_manager().resetAnim();

    ::opengl::scoped::name_pusher type (DoodadName);
    ::opengl::scoped::name_pusher dummy (0);
    for ( std::map<int, ModelInstance*>::iterator it (mModelInstances.begin())
        ; it != mModelInstances.end()
        ; ++it
        )
    {
      if (it->second->is_visible (mapdrawdistance, frustum, camera))
      {
        it->second->draw_for_selection();
      }
    }
  }

  GLuint minDist = 0xFFFFFFFF;
  GLNameEntry* minEntry = nullptr;

  size_t offset = 0;

  const GLint hits (gl.renderMode (GL_RENDER));
  for (GLint hit (0); hit < hits; ++hit)
  {
    GLNameEntry* entry = reinterpret_cast<GLNameEntry*>( &selection_buffer[offset] );

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
    if (minEntry->stack.type == MapObjName)
    {
      return selection_type
        (selection_names().findEntry (minEntry->stack.uniqueId)->data.wmo);
    }
    else if (minEntry->stack.type == DoodadName)
    {
      return selection_type
        (selection_names().findEntry (minEntry->stack.uniqueId)->data.model);
    }
    else if (minEntry->stack.type == MapTileName)
    {
      return selection_type
        ( selected_chunk_type ( selection_names().findEntry (minEntry->stack.chunk)->data.mapchunk
                              , minEntry->stack.triangle
                              )
        );
    }
  }

  return boost::none;
}

void World::advance_times ( const float& seconds
                          , const float& time_of_day_speed_factor
                          )
{
  time += time_of_day_speed_factor * seconds;
}

void World::tick (float dt)
{
  load_tiles_around ( camera.x() / TILESIZE
                    , camera.z() / TILESIZE
                    //! \todo Something based on viewing distance.
                    , 2
                    );

  while (dt > 0.1f) {
    noggit::app().model_manager().updateEmitters(0.1f);
    dt -= 0.1f;
  }
  noggit::app().model_manager().updateEmitters(dt);
}

unsigned int World::getAreaID() const
{
  const int mtx = camera.x() / TILESIZE;
  const int mtz = camera.z() / TILESIZE;
  const int mcx = fmod(camera.x(), TILESIZE) / CHUNKSIZE;
  const int mcz = fmod(camera.z(), TILESIZE) / CHUNKSIZE;

  if (!tileLoaded (mtz, mtx))
  {
    return 0;
  }

  const MapChunk *curChunk (mTiles[mtz][mtx].tile->getChunk(mcx, mcz));
  if(!curChunk)
    return 0;

  return curChunk->header.areaid;
}

void World::clearHeight(int x, int z)
{
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile;
      curTile = mTiles[z][x].tile.get();
      if(curTile == 0) continue;
      setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      if(curChunk == 0) continue;

      curChunk->vmin.y (9999999.0f);
      curChunk->vmax.y (-9999999.0f);

      for(int k=0; k < mapbufsize; ++k)
      {
        curChunk->mVertices[k].y (0.0f);

        curChunk->vmin.y (std::min(curChunk->vmin.y(),curChunk-> mVertices[k].y()));
        curChunk->vmax.y (std::max(curChunk->vmax.y(), curChunk->mVertices[k].y()));
      }

      gl.bindBuffer(GL_ARRAY_BUFFER, curChunk->vertices);
      gl.bufferData(GL_ARRAY_BUFFER, sizeof(curChunk->mVertices), curChunk->mVertices, GL_STATIC_DRAW);
    }
  }

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile = mTiles[z][x].tile.get();
      if(curTile == 0) continue;
      setChanged(z,x);
      curTile->getChunk(j, i)->update_normal_vectors();
    }
  }
}

void World::clearAllModelsOnADT(int x,int z)
{
  // get the adt
  MapTile *curTile;
  curTile = mTiles[z][x].tile.get();
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

void World::setAreaID (int id, const ::math::vector_3d& position)
{
  const int mtx (position.x() / TILESIZE);
  const int mtz (position.z() / TILESIZE);
  const int mcx (fmod (position.x(), TILESIZE) / CHUNKSIZE);
  const int mcz (fmod (position.z(), TILESIZE) / CHUNKSIZE);

  setAreaID (id, mtx, mtz, mcx, mcz);
}

void World::setAreaID(int id, int x, int z , int _cx, int _cz)
{

  // set the Area ID on a tile x,z on the chunk cx,cz
  MapTile *curTile;
  curTile = mTiles[z][x].tile.get();
  if(curTile == 0) return;
  setChanged(z,x);
  MapChunk *curChunk = curTile->getChunk(_cx, _cz);

  if(curChunk == 0) return;

  curChunk->header.areaid = id;
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

  gl.clear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  gl.enable (GL_BLEND);

  gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gl.enableClientState (GL_COLOR_ARRAY);
  gl.disableClientState (GL_NORMAL_ARRAY);
  gl.disableClientState (GL_TEXTURE_COORD_ARRAY);
  gl.disable (GL_CULL_FACE);
  gl.depthMask (GL_FALSE);

  {
    opengl::scoped::matrix_pusher const scale_matrix;
    gl.scalef (zoom, zoom, 1.0f);

    {
      opengl::scoped::matrix_pusher const translate_matrix;
      gl.translatef (-camera.x() / CHUNKSIZE, -camera.z() / CHUNKSIZE, 0.0f);

      //! \todo Only iterate over those intersecting?
      for (size_t j (0); j < 64; ++j)
      {
        for (size_t i (0); i < 64; ++i)
        {
          if (tileLoaded (j, i))
          {
            const MapTile* tile (mTiles[j][i].tile.get());
            const QRectF map_rect ( tile->xbase / CHUNKSIZE
                                  , tile->zbase / CHUNKSIZE
                                  , TILESIZE / CHUNKSIZE
                                  , TILESIZE / CHUNKSIZE
                                  );

            if (drawing_rect.intersects (map_rect))
            {
              tile->drawTextures ( drawing_rect.intersected (map_rect)
                                               .translated (-map_rect.topLeft())
                                 );
            }
          }
        }
      }
    }

    if (draw_lines)
    {
      gl.translatef(fmod(-camera.x()/CHUNKSIZE,16), fmod(-camera.z()/CHUNKSIZE,16),0);
      for(float x = -32.0f; x <= 48.0f; x += 1.0f)
      {
        if( static_cast<int>(x) % 16 )
          gl.color4f(1.0f,0.0f,0.0f,0.5f);
        else
          gl.color4f(0.0f,1.0f,0.0f,0.5f);
        gl.begin(GL_LINES);
        gl.vertex3f(-32.0f,x,-1);
        gl.vertex3f(48.0f,x,-1);
        gl.vertex3f(x,-32.0f,-1);
        gl.vertex3f(x,48.0f,-1);
        gl.end();
      }
    }
  }

  gl.disableClientState (GL_COLOR_ARRAY);

  gl.enableClientState (GL_NORMAL_ARRAY);
  gl.enableClientState (GL_TEXTURE_COORD_ARRAY);
}

boost::optional<float> World::get_height ( const float& x
                                         , const float& z
                                         ) const
{
  const int newX (x / TILESIZE);
  const int newZ (z / TILESIZE);

  if (!tileLoaded (newZ, newX))
  {
    return boost::none;
  }

  return mTiles[newZ][newX].tile->get_height (x, z);
}

struct chunk_identifier
{
  size_t _tile_x;
  size_t _tile_y;
  size_t _chunk_x;
  size_t _chunk_y;

  chunk_identifier ( const size_t& tile_x, const size_t& tile_y
                   , const size_t& chunk_x, const size_t& chunk_y
                   )
    : _tile_x (tile_x)
    , _tile_y (tile_y)
    , _chunk_x (chunk_x)
    , _chunk_y (chunk_y)
  { }
};

typedef std::vector<chunk_identifier> changed_chunks_type;

void World::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  changed_chunks_type changed_chunks;

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
            {
              changed_chunks.push_back (chunk_identifier (i, j, tx, ty));
              setChanged( j, i );
            }
          }
        }
      }
    }
  }

  for ( changed_chunks_type::const_iterator it (changed_chunks.begin())
      ; it != changed_chunks.end()
      ; ++it
      )
  {
    mTiles[it->_tile_y][it->_tile_x].tile->
      getChunk (it->_chunk_y, it->_chunk_x)->update_normal_vectors();
  }
}

void World::flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType)
{
  changed_chunks_type changed_chunks;

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
            {
              changed_chunks.push_back (chunk_identifier (i, j, tx, ty));
              setChanged( j, i );
            }
          }
        }
      }
    }
  }

  for ( changed_chunks_type::const_iterator it (changed_chunks.begin())
      ; it != changed_chunks.end()
      ; ++it
      )
  {
    mTiles[it->_tile_y][it->_tile_x].tile->
      getChunk (it->_chunk_y, it->_chunk_x)->update_normal_vectors();
  }
}

void World::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
  changed_chunks_type changed_chunks;

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
            {
              changed_chunks.push_back (chunk_identifier (i, j, tx, ty));
              setChanged( j, i );
            }
          }
        }
      }
    }
  }

  for ( changed_chunks_type::const_iterator it (changed_chunks.begin())
      ; it != changed_chunks.end()
      ; ++it
      )
  {
    mTiles[it->_tile_y][it->_tile_x].tile->
      getChunk (it->_chunk_y, it->_chunk_x)->update_normal_vectors();
  }
}

bool World::paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture)
{
  int const x_lower (static_cast<int> ((x - Brush.getRadius()) / TILESIZE));
  int const x_upper (static_cast<int> ((x + Brush.getRadius()) / TILESIZE));
  int const z_lower (static_cast<int> ((z - Brush.getRadius()) / TILESIZE));
  int const z_upper (static_cast<int> ((z + Brush.getRadius()) / TILESIZE));

  bool succ = false;

  for (int j (z_lower); j <= z_upper; ++j)
  {
    for (int i (x_lower); i <= x_upper; ++i)
    {
      if( tileLoaded( j, i ) )
      {
        int const x_chunk_lower (std::max (0, static_cast<int> ((x - Brush.getRadius()) / CHUNKSIZE) - i * 16));
        int const x_chunk_upper (std::min (16, static_cast<int> ((x + Brush.getRadius()) / CHUNKSIZE) - i * 16));
        int const z_chunk_lower (std::max (0, static_cast<int> ((z - Brush.getRadius()) / CHUNKSIZE) - j * 16));
        int const z_chunk_upper (std::min (16, static_cast<int> ((z + Brush.getRadius()) / CHUNKSIZE) - j * 16));

        for (int ty (x_chunk_lower); ty <= x_chunk_upper; ++ty)
        {
          for (int tx (z_chunk_lower); tx <= z_chunk_upper; ++tx)
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

void World::overwriteTextureAtCurrentChunk(float x, float z, noggit::scoped_blp_texture_reference oldTexture, noggit::scoped_blp_texture_reference newTexture)
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

void World::addHole (float x, float z, float h, bool whole_chunk)
{
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
            // check if the cursor is not undermap
            if(chunk->getMinHeight() <= h + 1.0f && chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              setChanged(x, z);

              if (whole_chunk)
              {
                chunk->make_all_holes();
              }
              else
              {
                int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
                int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
                chunk->addHole (k, l);
              }
            }
          }
        }
      }
    }
  }
}

void World::removeHole (float x, float z, bool whole_chunk)
{
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
              setChanged(x, z);

              if (whole_chunk)
              {
                chunk->remove_all_holes();
              }
              else
              {
                int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
                int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
                chunk->removeHole (k, l);
              }
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
  gl.enable(GL_BLEND);
  gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.readBuffer(GL_BACK);

  gl.enableClientState(GL_COLOR_ARRAY);
  gl.disableClientState(GL_NORMAL_ARRAY);
  gl.disableClientState(GL_TEXTURE_COORD_ARRAY);

  for(int y=0;y<64;y++)
  {
    for(int x=0;x<64;x++)
    {
      if( !( mTiles[y][x].flags & 1 ) )
      {
        continue;
      }

      ATile=loadTile(x,y);
      gl.clear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

      {
        opengl::scoped::matrix_pusher const matrix_pusher;

        gl.scalef(0.08333333f,0.08333333f,1.0f);

        //gl.translatef(-camera.x()/CHUNKSIZE,-camera.z()/CHUNKSIZE,0);
        gl.translatef( x * -16.0f - 8.0f, y * -16.0f - 8.0f, 0.0f );

        ATile->drawTextures (QRect (0, 0, 16, 16));
      }

  //! \todo Fix these two lines. THEY ARE VITAL!
//    gl.readPixels (video.xres()/2-128, video.yres()/2-128, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, image);
//      swapBuffers();

    std::stringstream ss;
    ss << basename.c_str() << "_map_" << x << "_" << y << ".raw";
    fid=fopen(ss.str().c_str(),"wb");
      fwrite(image,256*3,256,fid);
      fclose(fid);
    }
  }

  gl.disableClientState(GL_COLOR_ARRAY);

  gl.enableClientState(GL_NORMAL_ARRAY);
  gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
}

void World::deleteModelInstance( int pUniqueID )
{
  std::map<int, ModelInstance*>::iterator it = mModelInstances.find( pUniqueID );
  setChanged( it->second->pos.x(), it->second->pos.z() );
  delete it->second;
  mModelInstances.erase( it );
}

void World::deleteWMOInstance( int pUniqueID )
{
  std::map<int, WMOInstance *>::iterator it = mWMOInstances.find( pUniqueID );
  setChanged( it->second->pos.x(), it->second->pos.z() );
  mWMOInstances.erase( it );
}

void World::addModel ( const nameEntry& entry
                     , ::math::vector_3d newPos
                     , bool size_randomization
                     , bool position_randomization
                     , bool rotation_randomization
                     )
{
  if( entry.type == eEntry_Model )
    addM2 ( entry.data.model->model->_filename
          , newPos
          , size_randomization
          , position_randomization
          , rotation_randomization
          );
  else if( entry.type == eEntry_WMO )
    addWMO( entry.data.wmo->wmo->_filename, newPos );
}

void World::addM2 ( std::string const& path
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

  ModelInstance *newModelis = new ModelInstance(this, path);
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

void World::addWMO (std::string const& path, ::math::vector_3d newPos )
{
  const int lMaxUID = std::max( ( mModelInstances.empty() ? 0 : mModelInstances.rbegin()->first + 1 ),
                           ( mWMOInstances.empty() ? 0 : mWMOInstances.rbegin()->first + 1 ) );

  WMOInstance *newWMOis = new WMOInstance(this, path);
  newWMOis->pos = newPos;
  newWMOis->mUniqueID = lMaxUID;
  newWMOis->recalc_extents();
  mWMOInstances.insert( std::pair<int,WMOInstance *>( lMaxUID, newWMOis ));
  setChanged(newPos.x(),newPos.z());
}

static int tile_below_camera (const float& position)
{
  return ::math::bounded_nearest<int> ((position - (TILESIZE / 2)) / TILESIZE);
}

void World::setChanged (float x, float z)
{
  const int column (tile_below_camera (z));
  const int row (tile_below_camera (x));

  setChanged (column, row);
}

void World::setChanged(int x, int z)
{
  assert (oktile (x, z));
  _tile_got_modified[x][z] = true;
}

void World::unsetChanged(int x, int z)
{
  assert (oktile (x, z));
  _tile_got_modified[x][z] = false;
}

bool World::getChanged(int x, int z) const
{
  assert (oktile (x, z));
  return _tile_got_modified[x][z];
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

const unsigned int& World::getMapID() const
{
  return mMapId;
}

void World::moveHeight(int x, int z, const float& heightDelta)
{
  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile;
      curTile = mTiles[z][x].tile.get();
      if(curTile == 0) continue;
      setChanged(z,x);
      MapChunk *curChunk = curTile->getChunk(j, i);
      if(curChunk == 0) continue;

      for(int k=0; k < mapbufsize; ++k)
      {
        curChunk->mVertices[k].y (curChunk->mVertices[k].y() + heightDelta);
      }

      curChunk->vmin.y (curChunk->vmin.y() + heightDelta);
      curChunk->vmax.y (curChunk->vmax.y() + heightDelta);

      gl.bindBuffer(GL_ARRAY_BUFFER, curChunk->vertices);
      gl.bufferData(GL_ARRAY_BUFFER, sizeof(curChunk->mVertices), curChunk->mVertices, GL_STATIC_DRAW);
    }
  }

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapTile *curTile = mTiles[z][x].tile.get();
      if(curTile == 0) continue;
      setChanged(z,x);
      curTile->getChunk(j, i)->update_normal_vectors();
    }
  }

}

void World::setBaseTexture( int x, int z, noggit::scoped_blp_texture_reference texture )
{
  MapTile *curTile;
  curTile = mTiles[z][x].tile.get();
  if(curTile == 0) return;

  for (int j=0; j<16; ++j)
  {
    for (int i=0; i<16; ++i)
    {
      MapChunk *curChunk = curTile->getChunk(j, i);
      curChunk->eraseTextures();
      curChunk->addTexture( texture );
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

#define accessor(T_, name_)                                         \
  void World::setWater ## name_ (int x, int y, T_ value)            \
  {                                                                 \
    if (tileLoaded (y, x))                                          \
    {                                                               \
      mTiles[y][x].tile->_water.set ## name_ (value);               \
      setChanged (y, x);                                            \
    }                                                               \
  }                                                                 \
  boost::optional<T_> World::getWater ## name_ (int x, int y) const \
  {                                                                 \
    if (tileLoaded (y, x))                                          \
    {                                                               \
      return mTiles[y][x].tile->_water.get ## name_();              \
    }                                                               \
    return boost::none;                                             \
  }

  accessor (float, Height)
  accessor (unsigned char, Trans)
  accessor (int, Type)

#undef accessor

void World::autoGenWaterTrans (int x, int y, int factor)
{
  if (tileLoaded (y, x))
  {
    mTiles[y][x].tile->_water.autoGen (factor);
    setChanged (y, x);
  }
}
