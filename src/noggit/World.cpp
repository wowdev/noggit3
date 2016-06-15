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

#include <QOpenGLFramebufferObject>
#include <QSettings>

#include <math/bounded_nearest.h>
#include <math/random.h>
#include <math/vector_2d.h>

#include <opengl/call_list.h>
#include <opengl/context.h>
#include <opengl/matrix.h>
#include <opengl/scoped.h>
#include <opengl/settings_saver.h>
#include <opengl/shader.hpp>

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
#include <noggit/ui/cursor_selector.h>
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
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth_test;
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting;

    //! \todo This should be passed in!
    QSettings settings;
    gl.color4f ( settings.value ("cursor/red", 1.0f).toFloat()
              , settings.value ("cursor/green", 1.0f).toFloat()
              , settings.value ("cursor/blue", 1.0f).toFloat()
              , settings.value ("cursor/alpha", 1.0f).toFloat()
              );

    opengl::scoped::matrix_pusher matrix;

    gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, position).transposed());

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

      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, position).transposed());
      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, math::vector_3d {90.0f, 0.0f, 0.0f}).transposed());

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

      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, position).transposed());

      draw_sphere (0.3f);
    }
  }
}

namespace shader
{
  const std::string wmo_vert = R"code(
#version 110

attribute vec4 position;
attribute vec2 texcoord;

uniform mat4 model_view;
uniform mat4 projection;

varying vec2 vary_texcoord;

void main()
{
  vary_texcoord = texcoord;
  gl_Position = projection * model_view * position;
}
)code";

  const std::string wmo_frag = R"code(
#version 110

uniform sampler2D texture;
uniform float alpha_threshold;

varying vec2 vary_texcoord;

void main()
{
  vec4 diffuse = texture2D(texture, vary_texcoord);

  if(alpha_threshold >= 0.0 && diffuse.a <= alpha_threshold)
  {
    discard;
    return;
  }

  gl_FragColor = diffuse;
}
)code";

  const std::string mfbo_vert = R"code(
#version 110

attribute vec4 position;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * model_view * position;
}
)code";

  const std::string mfbo_frag = R"code(
#version 110

uniform vec4 color;

void main()
{
  gl_FragColor = color;
}
)code";

  const std::string mcnk_vert = R"code(
#version 110

attribute vec4 position;
attribute vec3 normal;
attribute vec2 texcoord;

uniform mat4 model_view;
uniform mat4 projection;

varying vec4 vary_position;
varying vec2 vary_texcoord;
varying vec3 vary_normal;

void main()
{
  gl_Position = projection * model_view * position;
  //! \todo gl_NormalMatrix deprecated
  vary_normal = normalize (gl_NormalMatrix * normal);
  vary_position = position;
  vary_texcoord = texcoord;
}
)code";

  const std::string mcnk_frag = R"code(
#version 110

uniform bool draw_area_id_overlay;
//! \todo draw triangle selection cursor
// uniform bool draw_triangle_selection_cursor;
uniform bool draw_terrain_height_contour;
uniform bool is_impassable_chunk;
uniform bool mark_impassable_chunks;
uniform vec3 area_id_color;
uniform vec3 shadow_color;
uniform int layer_count;

uniform bool draw_cursor_circle;
uniform vec3 cursor_position;
uniform float outer_cursor_radius;
uniform vec4 cursor_color;

uniform mat4 model_view;

uniform sampler2D shadow_map;

uniform sampler2D textures[4];
uniform sampler2D alphamaps[3];

varying vec4 vary_position;
varying vec2 vary_texcoord;
varying vec3 vary_normal;

const float contour_height_delta = 2.0;

// glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
vec4 blend_by_alpha (in vec4 source, in vec4 dest)
{
  return source * source.w + dest * (1.0 - source.w);
}

vec4 phong_lighting(in vec4 diffuse)
{
  // Implementing Phong Shader (for one Point-Light)
  // https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php

  vec3 N = vary_normal;
  vec3 v = (model_view * vary_position).xyz;

  //! \todo gl_LightSource deprecated
  vec3 L = normalize (gl_LightSource[0].position.xyz - v);
  vec3 E = normalize (-v);
  vec3 R = normalize (-reflect (L, N));

  //! \todo gl_FrontLightProduct deprecated
  vec4 Iamb = gl_FrontLightProduct[0].ambient;
  vec4 Idiff = clamp (diffuse * max (dot (N, L), 0.0), 0.0, 1.0);
  vec4 Ispec = clamp (gl_FrontLightProduct[0].specular * pow (max (dot (R, E),0.0), 0.3 * gl_FrontMaterial.shininess), 0.0, 1.0);

  //! \todo gl_FrontLightModelProduct deprecated
  return gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;
}

vec4 texture_blend() {
  if(layer_count == 0)
    return vec4 (1.0, 1.0, 1.0, 1.0);

  vec4 color = vec4 (0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < layer_count; ++i)
  {
    float alpha = 1.0;
    vec4 texture_color = texture2D (textures[i], vary_texcoord * 8.0);

    if (i != 0)
    {
      alpha = texture2D (alphamaps[i - 1], vary_texcoord).a;
    }

    color = blend_by_alpha (vec4 (texture_color.rgb, alpha), color);
  }

  return color;
}

void main()
{
  if (draw_terrain_height_contour && abs (mod (vary_position.y, contour_height_delta)) <= 0.25)
  {
    gl_FragColor = vec4 (0.0, 0.0, 0.0, 1.0);
    return;
  }

  //! \todo is selected triangle in triangle selection cursor mode
  // if (draw_triangle_selection_cursor && is_selected_triangle)
  // {
  //   gl_FragColor = vec4 (1.0, 1.0, 0.0, 1.0);
  //   return;
  // }

  //! \todo this is quite bright. pretty sure its a gamma issue
  gl_FragColor = phong_lighting(texture_blend());

  gl_FragColor = blend_by_alpha (vec4 (shadow_color, texture2D (shadow_map, vary_texcoord).a), gl_FragColor);

  if (mark_impassable_chunks && is_impassable_chunk)
  {
    gl_FragColor = blend_by_alpha (vec4 (1.0, 1.0, 1.0, 0.6), gl_FragColor);
  }

  if (draw_area_id_overlay)
  {
    gl_FragColor = blend_by_alpha (vec4 (area_id_color, 0.7), gl_FragColor);
  }

  if (draw_cursor_circle && abs (distance (vary_position.xz, cursor_position.xz) - outer_cursor_radius) <= 0.1)
  {
    gl_FragColor = blend_by_alpha (cursor_color, gl_FragColor);
  }
}
)code";

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
  : time( 1450 )
  , camera( ::math::vector_3d( 0.0f, 0.0f, 0.0f ) )
  , lookat( ::math::vector_3d( 0.0f, 0.0f, 0.0f ) )
  , outdoorLightStats( OutdoorLightStats() )
  , _initialized_display (false)
  , detailtexcoords( 0 )
  , alphatexcoords( 0 )
  , mMapId( 0xFFFFFFFF )
  , basename( name )
  , _selection_names()
  , _map_index(this, name)
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
    }
  }

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

void World::outdoorLighting()
{
  ::math::vector_4d black(0,0,0,0);
  gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, math::vector_4d {skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1.0f});

  gl.lightfv(GL_LIGHT0, GL_AMBIENT, black);
  gl.lightfv(GL_LIGHT0, GL_DIFFUSE, math::vector_4d {skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * outdoorLightStats.dayIntensity, 1.0f});
  gl.lightfv(GL_LIGHT0, GL_POSITION, math::vector_4d {convert_rotation (outdoorLightStats.dayDir), 0.0f});

  /*
  gl.lightfv(GL_LIGHT1, GL_AMBIENT, black);
  gl.lightfv(GL_LIGHT1, GL_DIFFUSE, math::vector_4d {skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * outdoorLightStats.nightIntensity, 1.0f});
  gl.lightfv(GL_LIGHT1, GL_POSITION, math::vector_4d {convert_rotation (outdoorLightStats.nightDir), 0.0f});*/
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

namespace
{
  enum row_type { inner, outer };
  void add (std::vector<math::vector_2d>& texcoords, std::size_t x, std::size_t y, row_type type)
  {
    static float const per_step (1.0f / 8.0f);
    static math::vector_2d const inner_inset (per_step / 2.0f, per_step / 2.0f);

    if (type == inner)
    {
      texcoords.emplace_back (inner_inset + math::vector_2d (x, y) * per_step);
    }
    else
    {
      texcoords.emplace_back (math::vector_2d (x, y) * per_step);
    }
  }
  void add_row (std::vector<math::vector_2d>& texcoords, std::size_t y, row_type type)
  {
    for (std::size_t x (0); x < (type == inner ? 8 : 9); ++x)
    {
      add (texcoords, x, y, type);
    }
  }

  std::vector<math::vector_2d> make_texcoords()
  {
    std::vector<math::vector_2d> texcoords;
    texcoords.reserve (9 * 9 + 8 * 8);

    for (std::size_t y (0); y < 8; ++y)
    {
      add_row (texcoords, y, outer);
      add_row (texcoords, y, inner);
    }
    add_row (texcoords, 8, outer);

    return texcoords;
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
        if ( !(i==cx&&j==cz) && noggit::map_index::ok_tile(i,j) && lowrestiles[j][i])
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

    if (flags & TERRAIN)
    {
      gl.bindBuffer (GL_ARRAY_BUFFER, 0);
      gl.bindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

      static opengl::program const program {
        { GL_VERTEX_SHADER,   shader::mcnk_vert },
        { GL_FRAGMENT_SHADER, shader::mcnk_frag }
      };

      opengl::scoped::use_program mcnk_shader {program};

      mcnk_shader.uniform ("model_view", opengl::matrix::model_view());
      mcnk_shader.uniform ("projection", opengl::matrix::projection());

      mcnk_shader.uniform ("shadow_color", skies->colorSet[SHADOW_COLOR]);

      static auto const texcoords (make_texcoords());
      mcnk_shader.attrib ("texcoord", texcoords);

      mcnk_shader.uniform ("draw_area_id_overlay", !!(flags & AREAID));
      mcnk_shader.uniform ("draw_terrain_height_contour", !!(flags & HEIGHTCONTOUR));
      mcnk_shader.uniform ("mark_impassable_chunks", !!(flags & MARKIMPASSABLE));

      QSettings settings;
      mcnk_shader.uniform ("draw_cursor_circle", selected_item && noggit::selection::is_chunk (*selected_item) && noggit::app().setting ("cursor/type", 1).toInt() == noggit::ui::cursor_type::circle);
      mcnk_shader.uniform ("cursor_position", _exact_terrain_selection_position);
      mcnk_shader.uniform ("outer_cursor_radius", outer_cursor_radius);
      mcnk_shader.uniform ( "cursor_color"
                          , math::vector_4d ( settings.value ("cursor/red", 1.0f).toFloat()
                                            , settings.value ("cursor/green", 1.0f).toFloat()
                                            , settings.value ("cursor/blue", 1.0f).toFloat()
                                            , settings.value ("cursor/alpha", 1.0f).toFloat()
                                            )
                          );

      //! \todo draw triangle selection cursor
      // selected indices = mapstrip2[noggit::selection::selected_polygon (*selected_item) + 0…2]
      // mcnk_shader.uniform ( "draw_triangle_selection_cursor"
      //                     , !(flags & NOCURSOR)
      //                     //! \todo This actually should be an enum.
      //                     && noggit::app().setting ("cursor/type", 1).toInt() == noggit::ui::cursor_type::triangle
      //                     );

      for (int j (0); j < 64; ++j)
      {
        for (int i (0); i < 64; ++i)
        {
          if (_map_index.tile_loaded (j, i))
          {
            _map_index.tile (j, i)->draw
              (mcnk_shader, mapdrawdistance, frustum, camera, selected_item);
          }
        }
      }
    }

    // Selection circle
    //! \todo raycasting instead of readPixels and depth buffer

    GLint viewport[4];
    gl.getIntegerv (GL_VIEWPORT, viewport);

    int const win_x (mouse_position.x());
    int const win_y (static_cast<float> (viewport[3]) - mouse_position.y());
    float win_z;

    if (QSurfaceFormat::defaultFormat().samples() != -1)
    {
      QOpenGLFramebufferObject downsampled_fbo (1, 1, QOpenGLFramebufferObject::CombinedDepthStencil);

      QOpenGLFramebufferObject::blitFramebuffer
        (&downsampled_fbo, {0, 0, 1, 1}, nullptr, {win_x, win_y, 1, 1}, GL_DEPTH_BUFFER_BIT);

      downsampled_fbo.bind();
      gl.readPixels (0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);
    }
    else
    {
      gl.readPixels (win_x, win_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);
    }

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

    if (selected_item && noggit::selection::is_chunk (*selected_item))
    {
      if (!(flags & NOCURSOR))
      {
        QSettings settings;
        //! \todo This actually should be an enum. And should be passed into this method.
        const int cursor_type (settings.value ("cursor/type", 1).toInt());
        if(cursor_type == noggit::ui::cursor_type::disk)
          renderDisk_convenient ( _exact_terrain_selection_position
                                , outer_cursor_radius
                                );
        else if(cursor_type == noggit::ui::cursor_type::sphere)
          renderSphere_convenient ( _exact_terrain_selection_position
                                  , outer_cursor_radius
                                  );
      }
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
          if( _map_index.tile_loaded( j, i ) )
          {
            _map_index.tile( j, i )->drawLines ( flags & HOLELINES
                                         , mapdrawdistance
                                         , frustum
                                         , camera
                                         );
          }
        }
      }

      gl.bindBuffer (GL_ARRAY_BUFFER, 0);
      gl.bindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

      static opengl::program const program {
        { GL_VERTEX_SHADER,   shader::mfbo_vert },
        { GL_FRAGMENT_SHADER, shader::mfbo_frag }
      };

      opengl::scoped::use_program mfbo_shader {program};

      mfbo_shader.uniform ("model_view", opengl::matrix::model_view());
      mfbo_shader.uniform ("projection", opengl::matrix::projection());

      for( int j = 0; j < 64; ++j )
      {
        for( int i = 0; i < 64; ++i )
        {
          if( _map_index.tile_loaded( j, i ) )
          {
            _map_index.tile( j, i )->drawMFBO (mfbo_shader);
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
    gl.bindBuffer (GL_ARRAY_BUFFER, 0);
    gl.bindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

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
    }


    if(flags & WMODOODAS)
    {
      for (std::map<int, WMOInstance *>::iterator it = mWMOInstances.begin (); it != mWMOInstances.end (); ++it)
        it->second->draw_doodads( flags & FOG
                                , (flags & FOG) ? fog_distance : mapdrawdistance
                                , fog_distance
                                , frustum
                                , camera
                                );
    }

    // WMOs / map objects
    if (flags & DRAWWMO)
    {
      opengl::scoped::bool_setter<GL_BLEND, GL_FALSE> const blend;
      opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull_face;

      static opengl::program const program {
        { GL_VERTEX_SHADER,   shader::wmo_vert },
        { GL_FRAGMENT_SHADER, shader::wmo_frag }
      };

      opengl::scoped::use_program wmo_shader { program };

      wmo_shader.uniform ("projection", opengl::matrix::projection ());

      for( std::map<int, WMOInstance *>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
        it->second->draw ( wmo_shader
                         , flags & FOG
                         , skies->hasSkies()
                         , (flags & FOG) ? fog_distance : mapdrawdistance
                         , fog_distance
                         , frustum
                         , camera
                         , selected_item
                         );
    }

    outdoorLights (true);
    setupFog (flags & FOG, fog_distance);

    gl.color4f (1.0f, 1.0f, 1.0f, 1.0f);
    gl.disable (GL_CULL_FACE);

    gl.disable (GL_BLEND);
    gl.disable (GL_ALPHA_TEST);
    gl.enable (GL_LIGHTING);
  }

  setupFog (flags & FOG, fog_distance);

  gl.color4f (1, 1, 1, 1);
  gl.enable (GL_BLEND);

  //gl.color4f(1,1,1,1);
  gl.disable (GL_COLOR_MATERIAL);

  if(flags & WATER)
  {
    for( int j = 0; j < 64; ++j )
    {
      for( int i = 0; i < 64; ++i )
      {
        if( _map_index.tile_loaded( j, i ) )
        {
          _map_index.tile( j, i )->drawWater (skies.get());
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
        if( _map_index.tile_loaded( j, i ) )
        {
          _map_index.tile( j, i )->drawSelect ( mapdrawdistance
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
  _map_index.load_tiles_around ( camera.x() / TILESIZE
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

  if (!_map_index.tile_loaded (mtz, mtx))
  {
    return 0;
  }

  const MapChunk *curChunk (_map_index.tile(mtz, mtx)->getChunk(mcx, mcz));
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
      curTile = _map_index.tile(z, x);
      if(curTile == 0) continue;
      _map_index.set_changed(z,x);
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
      MapTile *curTile = _map_index.tile(z, x);
      if(curTile == 0) continue;
      _map_index.set_changed(z,x);
      curTile->getChunk(j, i)->update_normal_vectors();
    }
  }
}

void World::clearAllModelsOnADT(int x,int z)
{
  // get the adt
  MapTile *curTile;
  curTile = _map_index.tile(z, x);
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
  curTile = _map_index.tile(z, x);
  if(curTile == 0) return;
  _map_index.set_changed(z,x);
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
      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, {-camera.x() / CHUNKSIZE, -camera.z() / CHUNKSIZE, 0.0f}).transposed());

      //! \todo Only iterate over those intersecting?
      for (size_t j (0); j < 64; ++j)
      {
        for (size_t i (0); i < 64; ++i)
        {
          if (_map_index.tile_loaded (j, i))
          {
            const MapTile* tile (_map_index.tile( j, i ));
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
      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, {std::fmod (-camera.x()/CHUNKSIZE,16.0f), std::fmod (-camera.z()/CHUNKSIZE,16.0f), 0.0f}).transposed());
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

  if (!_map_index.tile_loaded (newZ, newX))
  {
    return boost::none;
  }

  return _map_index.tile(newZ, newX)->get_height (x, z);
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
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( _map_index.tile( j, i )->getChunk(ty,tx)->changeTerrain(x,z,change,radius,BrushType) )
            {
              changed_chunks.push_back (chunk_identifier (i, j, tx, ty));
              _map_index.set_changed( j, i );
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
    _map_index.tile(it->_tile_y, it->_tile_x)->
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
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( _map_index.tile( j, i )->getChunk(ty,tx)->flattenTerrain(x,z,h,remain,radius,BrushType) )
            {
              changed_chunks.push_back (chunk_identifier (i, j, tx, ty));
              _map_index.set_changed( j, i );
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
    _map_index.tile(it->_tile_y, it->_tile_x)->
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
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            if( _map_index.tile( j, i )->getChunk(ty,tx)->blurTerrain(x, z, remain, radius, BrushType) )
            {
              changed_chunks.push_back (chunk_identifier (i, j, tx, ty));
              _map_index.set_changed( j, i );
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
    _map_index.tile(it->_tile_y, it->_tile_x)->
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
      if( _map_index.tile_loaded( j, i ) )
      {
        int const x_chunk_lower (std::max (0, static_cast<int> ((x - Brush.getRadius()) / CHUNKSIZE) - i * 16));
        int const x_chunk_upper (std::min (16, static_cast<int> ((x + Brush.getRadius()) / CHUNKSIZE) - i * 16));
        int const z_chunk_lower (std::max (0, static_cast<int> ((z - Brush.getRadius()) / CHUNKSIZE) - j * 16));
        int const z_chunk_upper (std::min (16, static_cast<int> ((z + Brush.getRadius()) / CHUNKSIZE) - j * 16));

        for (int ty (x_chunk_lower); ty <= x_chunk_upper; ++ty)
        {
          for (int tx (z_chunk_lower); tx <= z_chunk_upper; ++tx)
          {
            if( _map_index.tile( j, i )->getChunk( ty, tx )->paintTexture( x, z, Brush, strength, pressure, texture ) )
            {
              succ |= true;
              _map_index.set_changed( j, i );
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
  _map_index.set_changed(x,z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;
  Log << "Erasing Textures at " << x << " and " << z;
  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = _map_index.tile( j, i )->getChunk( ty, tx );
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
  _map_index.set_changed(x,z);
  const size_t newX = x / TILESIZE;
  const size_t newZ = z / TILESIZE;
  Log << "Switching Textures at " << x << " and " << z;
  for( size_t j = newZ - 1; j < newZ + 1; ++j )
  {
    for( size_t i = newX - 1; i < newX + 1; ++i )
    {
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = _map_index.tile( j, i )->getChunk( ty, tx );
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
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = _map_index.tile( j, i )->getChunk( ty, tx );
            // check if the cursor is not undermap
            if(chunk->getMinHeight() <= h + 1.0f && chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              _map_index.set_changed(x, z);

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
      if( _map_index.tile_loaded( j, i ) )
      {
        for( size_t ty = 0; ty < 16; ++ty )
        {
          for( size_t tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = _map_index.tile( j, i )->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              _map_index.set_changed(x, z);

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
/*
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
*/
}

void World::deleteModelInstance( int pUniqueID )
{
  std::map<int, ModelInstance*>::iterator it = mModelInstances.find( pUniqueID );
  _map_index.set_changed( it->second->pos.x(), it->second->pos.z() );
  delete it->second;
  mModelInstances.erase( it );
}

void World::deleteWMOInstance( int pUniqueID )
{
  std::map<int, WMOInstance *>::iterator it = mWMOInstances.find( pUniqueID );
  _map_index.set_changed( it->second->pos.x(), it->second->pos.z() );
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
  _map_index.set_changed(newPos.x(), newPos.z());
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
  _map_index.set_changed(newPos.x(),newPos.z());
}

void World::setFlag( bool to, float x, float z)
{
  // set the inpass flag to selected chunk
  _map_index.set_changed(x, z);
  const int newX = x / TILESIZE;
  const int newZ = z / TILESIZE;

  for( int j = newZ - 1; j < newZ + 1; ++j )
  {
    for( int i = newX - 1; i < newX + 1; ++i )
    {
      if( _map_index.tile_loaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ++ty )
        {
          for( int tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = _map_index.tile( j, i )->getChunk( ty, tx );
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
      curTile = _map_index.tile(z, x);
      if(curTile == 0) continue;
      _map_index.set_changed(z,x);
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
      MapTile *curTile = _map_index.tile(z, x);
      if(curTile == 0) continue;
      _map_index.set_changed(z,x);
      curTile->getChunk(j, i)->update_normal_vectors();
    }
  }

}

void World::setBaseTexture( int x, int z, noggit::scoped_blp_texture_reference texture )
{
  MapTile *curTile;
  curTile = _map_index.tile(z, x);
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

#define accessor(T_, name_)                                         \
  void World::setWater ## name_ (int x, int y, T_ value)            \
  {                                                                 \
    if (_map_index.tile_loaded (y, x))                                          \
    {                                                               \
      _map_index.tile(y, x)->_water.set ## name_ (value);               \
      _map_index.set_changed (y, x);                                            \
    }                                                               \
  }                                                                 \
  boost::optional<T_> World::getWater ## name_ (int x, int y) const \
  {                                                                 \
    if (_map_index.tile_loaded (y, x))                                          \
    {                                                               \
      return _map_index.tile(y, x)->_water.get ## name_();              \
    }                                                               \
    return boost::none;                                             \
  }

  accessor (float, Height)
  accessor (unsigned char, Trans)
  accessor (int, Type)

#undef accessor

void World::autoGenWaterTrans (int x, int y, int factor)
{
  if (_map_index.tile_loaded (y, x))
  {
    _map_index.tile(y, x)->_water.autoGen (factor);
    _map_index.set_changed (y, x);
  }
}
