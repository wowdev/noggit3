// This file is part of Noggit3, licensed under GNU General Public License (version 3).


#include <noggit/World.h>

#include <math/frustum.hpp>
#include <noggit/Brush.h> // brush
#include <noggit/ChunkWater.hpp>
#include <noggit/ConfigFile.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Project.h>
#include <noggit/Settings.h>
#include <noggit/TextureManager.h>
#include <noggit/TileWater.hpp>// tile water
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/map_index.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/TexturingGUI.h>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/thread.hpp>

#include <algorithm>
#include <cassert>
#include <ctime>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>

namespace
{
  void render_line (math::vector_3d const& p1, math::vector_3d const& p2)
  {
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth_test;
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting;

    gl.lineWidth(2.5);

    gl.begin(GL_LINES);
    gl.vertex3fv (p1);
    gl.vertex3fv (p2);
    gl.end();
  }

  void draw_square(math::vector_3d const& pos, float size, float orientation)
  {
    float dx1 = size*cos(orientation) - size*sin(orientation);
    float dx2 = size*cos(orientation + math::constants::pi / 2) - size*sin(orientation + math::constants::pi / 2);
    float dz1 = size*sin(orientation) + size*cos(orientation);
    float dz2 = size*sin(orientation + math::constants::pi / 2) + size*cos(orientation + math::constants::pi / 2);

    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth_test;

    gl.begin(GL_QUADS);
    gl.vertex3f(pos.x + dx1, pos.y, pos.z + dz1);
    gl.vertex3f(pos.x + dx2, pos.y, pos.z + dz2);
    gl.vertex3f(pos.x - dx1, pos.y, pos.z - dz1);
    gl.vertex3f(pos.x - dx2, pos.y, pos.z - dz2);
    gl.vertex3f(pos.x + dx1, pos.y, pos.z + dz1);
    gl.end();
  }

  void render_square(math::vector_3d const& pos, float radius, float orientation, float inner_radius = 0.0f, bool useInnerRadius = true)
  {
    draw_square(pos, radius, orientation);

    if (useInnerRadius)
    {
      draw_square(pos, inner_radius, orientation);
    }
  }


  std::size_t const sphere_segments (15);
  void draw_sphere_point (int i, int j, float radius)
  {
    static math::radians const drho (math::constants::pi / sphere_segments);
    static math::radians const dtheta (2.0f * drho._);

    math::radians const rho (i * drho._);
    math::radians const theta (j * dtheta._);
    gl.vertex3f ( math::cos (theta) * math::sin (rho) * radius
                , math::sin (theta) * math::sin (rho) * radius
                , math::cos (rho) * radius
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
  void render_sphere (::math::vector_3d const& position, float radius, math::vector_4d const& color)
  {
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth_test;
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting;

    gl.color4f(color.x, color.y, color.z, color.w);

    opengl::scoped::matrix_pusher matrix;

    gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, position).transposed());

    draw_sphere (0.3f);
    draw_sphere (radius);
  }

  void draw_disk_point (float radius, math::radians& arc, math::radians const& angle, math::radians const& orientation)
  {
    float x = radius * math::sin(arc);
    float y = radius * math::cos(arc);
    float z = (y * math::cos(orientation) + x * math::sin(orientation)) * math::tan(angle);;
    gl.vertex3f (x, y, z);
  }
  void draw_disk (float radius, bool stipple, math::radians const& angle, math::radians const& orientation)
  {
    int const slices (std::max (35.0f, radius * 1.5f));
    static math::radians const max (2.0f * math::constants::pi);

    float const stride (max._ / slices);

    if (stipple)
    {
      gl.enable(GL_LINE_STIPPLE);
      gl.lineStipple(10, 0xAAAA);
    }

	  gl.lineWidth(3.0f);

      gl.begin (GL_LINE_LOOP);
      for (math::radians arc (0.0f); arc._ < max._; arc._ += stride)
      {
        draw_disk_point (radius, arc, angle, orientation);
      }
      gl.end();

	  gl.lineWidth(1.0f);

    if (stipple)
    {
      gl.disable(GL_LINE_STIPPLE);
    }
  }

  void render_disk( ::math::vector_3d const& position
                  , float radius
                  , math::vector_4d const& color
                  , bool stipple = false
                  , math::radians const& angle = math::radians(0.0f)
                  , math::radians const& orientation = math::radians(0.0f)
                  )
  {
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> lighting;

    {
      opengl::scoped::matrix_pusher matrix;
      opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth_test;
      gl.colorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
      opengl::scoped::bool_setter<GL_COLOR_MATERIAL, GL_TRUE> color_material;

      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, position).transposed());
      gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation_xyz, math::degrees::vec3 {math::degrees (90.0f), math::degrees (0.0f), math::degrees (0.0f)}).transposed());

      gl.color4f(color.x, color.y, color.z, color.w);

      draw_disk (radius, stipple, angle, orientation);
    }

    {
      opengl::scoped::matrix_pusher matrix;

      gl.multMatrixf(math::matrix_4x4(math::matrix_4x4::translation, position).transposed());

      draw_sphere(0.3f);
    }

  }
}

bool World::IsEditableWorld(int pMapId)
{
  std::string lMapName;
  try
  {
    DBCFile::Record map = gMapDB.getByID((unsigned int)pMapId);
    lMapName = map.getString(MapDB::InternalName);
  }
  catch (int)
  {
    LogError << "Did not find map with id " << pMapId << ". This is NOT editable.." << std::endl;
    return false;
  }

  std::stringstream ssfilename;
  ssfilename << "World\\Maps\\" << lMapName << "\\" << lMapName << ".wdt";

  if (!MPQFile::exists(ssfilename.str()))
  {
    Log << "World " << pMapId << ": " << lMapName << " has no WDT file!" << std::endl;
    return false;
  }

  MPQFile mf(ssfilename.str());

  //sometimes, wdts don't open, so ignore them...
  if (mf.isEof())
    return false;

  const char * lPointer = reinterpret_cast<const char*>(mf.getPointer());

  // Not using the libWDT here doubles performance. You might want to look at your lib again and improve it.
  const int lFlags = *(reinterpret_cast<const int*>(lPointer + 8 + 4 + 8));
  if (lFlags & 1)
    return false;

  const int * lData = reinterpret_cast<const int*>(lPointer + 8 + 4 + 8 + 0x20 + 8);
  for (int i = 0; i < 8192; i += 2)
  {
    if (lData[i] & 1)
      return true;
  }

  return false;
}

World::World(const std::string& name, int map_id)
  : mapIndex (name, map_id, this)
  , horizon(name)
  , mCurrentSelection()
  , SelectionMode(false)
  , mWmoFilename("")
  , mWmoEntry(ENTRY_MODF())
  , detailtexcoords(0)
  , alphatexcoords(0)
  , ol(nullptr)
  , animtime(0)
  , time(1450)
  , basename(name)
  , fogdistance(777.0f)
  , culldistance(fogdistance)
  , skies(nullptr)
  , outdoorLightStats(OutdoorLightStats())
{
  LogDebug << "Loading world \"" << name << "\"." << std::endl;
}

bool World::IsSelection(int pSelectionType)
{
  return HasSelection() && mCurrentSelection->which() == pSelectionType;
}

bool World::HasSelection()
{
  return !!mCurrentSelection;
}

void World::initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords)
{
  if (!*pDetailTexCoords && !*pAlphaTexCoords)
  {
    math::vector_2d temp[mapbufsize], *vt;
    float tx, ty;

    // init texture coordinates for detail map:
    vt = temp;
    const float detail_half = 0.5f * detail_size / 8.0f;
    for (int j = 0; j<17; ++j) {
      for (int i = 0; i<((j % 2) ? 8 : 9); ++i) {
        tx = detail_size / 8.0f * i;
        ty = detail_size / 8.0f * j * 0.5f;
        if (j % 2) {
          // offset by half
          tx += detail_half;
        }
        *vt++ = math::vector_2d(tx, ty);
      }
    }

    gl.genBuffers(1, pDetailTexCoords);
    gl.bufferData<GL_ARRAY_BUFFER> (*pDetailTexCoords, sizeof(temp), temp, GL_STATIC_DRAW);

    // init texture coordinates for alpha map:
    vt = temp;

    const float alpha_half = TEXDETAILSIZE / MINICHUNKSIZE;
    for (int j = 0; j<17; ++j) {
      for (int i = 0; i<((j % 2) ? 8 : 9); ++i) {
        tx = alpha_half * i *2.0f;
        ty = alpha_half * j;
        if (j % 2) {
          // offset by half
          tx += alpha_half;
        }
        *vt++ = math::vector_2d(tx, ty);
      }
    }

    gl.genBuffers(1, pAlphaTexCoords);
    gl.bufferData<GL_ARRAY_BUFFER> (*pAlphaTexCoords, sizeof(temp), temp, GL_STATIC_DRAW);
  }
}

void World::initDisplay()
{
  initGlobalVBOs(&detailtexcoords, &alphatexcoords);

  mapIndex.setAdt(false);

  if (mapIndex.hasAGlobalWMO())
  {
    WMOInstance inst(mWmoFilename, &mWmoEntry);
    //! \todo is this used? does it even make _any_ sense to set the camera position to the center of a wmo?
    // camera = inst.pos;
    mWMOInstances.emplace(mWmoEntry.uniqueID, std::move(inst));
  }
  else
  {
    _horizon_render = std::make_unique<noggit::map_horizon::render>(horizon);
  }

  skies = std::make_unique<Skies> (mapIndex._map_id);

  ol = std::make_unique<OutdoorLighting> ("World\\dnc.db");
}

void World::outdoorLighting()
{
  math::vector_4d black(0, 0, 0, 0);
  math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
  gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  float di = outdoorLightStats.dayIntensity;
  //float ni = outdoorLightStats.nightIntensity;

  math::vector_3d dd = outdoorLightStats.dayDir;
  // HACK: let's just keep the light source in place for now
  //math::vector_4d pos(-1, 1, -1, 0);
  math::vector_4d pos(-dd.x, -dd.z, dd.y, 0.0f);
  math::vector_4d col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1.0f);
  gl.lightfv(GL_LIGHT0, GL_AMBIENT, black);
  gl.lightfv(GL_LIGHT0, GL_DIFFUSE, col);
  gl.lightfv(GL_LIGHT0, GL_POSITION, pos);
}

void World::outdoorLights(bool on)
{
  float di = outdoorLightStats.dayIntensity;
  float ni = outdoorLightStats.nightIntensity;

  if (on) {
    math::vector_4d ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
    gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    if (di>0) {
      gl.enable(GL_LIGHT0);
    }
    else {
      gl.disable(GL_LIGHT0);
    }
    if (ni>0) {
      gl.enable(GL_LIGHT1);
    }
    else {
      gl.disable(GL_LIGHT1);
    }
  }
  else {
    math::vector_4d ambient(0, 0, 0, 1);
    gl.lightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    gl.disable(GL_LIGHT0);
    gl.disable(GL_LIGHT1);
  }
}

void World::setupFog (bool draw_fog)
{
  if (draw_fog) {

    //float fogdist = 357.0f; // minimum draw distance in wow
    //float fogdist = 777.0f; // maximum draw distance in wow

    float fogdist = fogdistance;
    float fogstart = 0.5f;

    culldistance = fogdist;

    //FOG_COLOR
    math::vector_4d fogcolor(skies->colorSet[FOG_COLOR], 1);
    gl.fogfv(GL_FOG_COLOR, fogcolor);
    //! \todo  retreive fogstart and fogend from lights.lit somehow
    gl.fogf(GL_FOG_END, fogdist);
    gl.fogf(GL_FOG_START, fogdist * fogstart);

    gl.enable(GL_FOG);
  }
  else {
    gl.disable(GL_FOG);
    culldistance = Settings::getInstance()->mapDrawDistance;
  }
}

void World::draw ( math::vector_3d const& cursor_pos
                 , math::vector_4d const& cursor_color
                 , int cursor_type
                 , float brushRadius
                 , bool show_unpaintable_chunks
                 , bool draw_contour
                 , float innerRadius
                 , math::vector_3d const& ref_pos
                 , float angle
                 , float orientation
                 , bool use_ref_pos
                 , bool angled_mode
                 , bool draw_paintability_overlay
                 , bool draw_chunk_flag_overlay
                 , bool draw_areaid_overlay
                 , editing_mode terrainMode
                 , math::vector_3d const& camera_pos
                 , bool draw_mfbo
                 , bool draw_wireframe
                 , bool draw_lines
                 , bool draw_terrain
                 , bool draw_wmo
                 , bool draw_water
                 , bool draw_wmo_doodads
                 , bool draw_models
                 , bool draw_model_animations
                 , bool draw_hole_lines
                 , bool draw_models_with_box
                 , std::unordered_set<WMO*> const& hidden_map_objects
                 , std::unordered_set<Model*> const& hidden_models
                 , std::map<int, misc::random_color>& area_id_colors
                 , bool draw_fog
                 , eTerrainType ground_editing_brush
                 , int water_layer
                 )
{
  if (!_display_initialized)
  {
    initDisplay();
    _display_initialized = true;
  }

  math::frustum const frustum
    (::opengl::matrix::model_view() * ::opengl::matrix::projection());

  bool hadSky = false;
  if (draw_wmo || mapIndex.hasAGlobalWMO())
  {
    for (std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it)
    {
      hadSky = it->second.wmo->drawSkybox ( camera_pos
                                          , it->second.extents[0]
                                          , it->second.extents[1]
                                          , draw_fog
                                          , animtime
                                          );
      if (hadSky)
      {
        break;
      }
    }
  }

  gl.enable(GL_CULL_FACE);
  gl.disable(GL_BLEND);
  opengl::texture::disable_texture();
  gl.disable(GL_DEPTH_TEST);
  gl.disable(GL_FOG);

  int daytime = static_cast<int>(time) % 2880;
  outdoorLightStats = ol->getLightStats(daytime);
  skies->initSky(camera_pos, daytime);

  if (!hadSky)
    hadSky = skies->drawSky ( camera_pos
                            , outdoorLightStats.nightIntensity
                            , draw_fog
                            , animtime
                            );

  // clearing the depth buffer only - color buffer is/has been overwritten anyway
  // unless there is no sky OR skybox
  GLbitfield clearmask = GL_DEPTH_BUFFER_BIT;
  if (!hadSky)   clearmask |= GL_COLOR_BUFFER_BIT;
  gl.clear(clearmask);

  opengl::texture::disable_texture();

  outdoorLighting();
  outdoorLights(true);

  gl.fogi(GL_FOG_MODE, GL_LINEAR);
  setupFog (draw_fog);

  // Draw verylowres heightmap
  if (draw_fog && draw_terrain) {
    _horizon_render->draw (&mapIndex, skies->colorSet[FOG_COLOR], culldistance, frustum, camera_pos);
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
  gl.color4f(1, 1, 1, 1);

  gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, math::vector_4d (0.1f, 0.1f, 0.1f, 0.1f));
  gl.materiali(GL_FRONT_AND_BACK, GL_SHININESS, 64);

  gl.lightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  gl.clientActiveTexture(GL_TEXTURE0);
  gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
  gl.texCoordPointer (detailtexcoords, 2, GL_FLOAT, 0, 0);

  gl.clientActiveTexture(GL_TEXTURE1);
  gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
  gl.texCoordPointer (alphatexcoords, 2, GL_FLOAT, 0, 0);

  gl.clientActiveTexture(GL_TEXTURE0);

  // height map w/ a zillion texture passes
  if (draw_terrain)
  {
    opengl::program const mcnk_program
    { { GL_VERTEX_SHADER
      , R"code(
#version 110

attribute vec4 position;
attribute vec3 normal;
attribute vec2 texcoord;
attribute vec2 alphacoord;
attribute vec3 mccv;

uniform mat4 model_view;
uniform mat4 projection;

varying vec4 vary_position;
varying vec2 vary_texcoord;
varying vec2 vary_alphacoord;
varying vec3 vary_normal;
varying vec3 vary_mccv;

void main()
{
  gl_Position = projection * model_view * position;
  //! \todo gl_NormalMatrix deprecated
  vary_normal = normalize (gl_NormalMatrix * normal);
  vary_position = position;
  vary_texcoord = texcoord;
  vary_alphacoord = alphacoord;
  vary_mccv = mccv;
}
)code"
      }
      , { GL_FRAGMENT_SHADER
      , R"code(
#version 110

uniform mat4 model_view;

uniform sampler2D shadow_map;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D alphamap;
uniform int layer_count;
uniform bool has_mccv;
uniform bool cant_paint;
uniform bool draw_areaid_overlay;
uniform vec4 areaid_color;
uniform bool draw_impassible_flag;
uniform bool draw_terrain_height_contour;
uniform bool draw_lines;

uniform bool draw_cursor_circle;
uniform vec3 cursor_position;
uniform float outer_cursor_radius;
uniform float inner_cursor_ratio;
uniform vec4 cursor_color;

varying vec4 vary_position;
varying vec2 vary_texcoord;
varying vec2 vary_alphacoord;
varying vec3 vary_normal;
varying vec3 vary_mccv;

const float TILESIZE = 533.33333f;
const float CHUNKSIZE = ((TILESIZE) / 16.0f);
const float UNITSIZE = (CHUNKSIZE / 8.0f);

vec4 blend_by_alpha (in vec4 source, in vec4 dest)
{
  return source * source.w + dest * (1.0 - source.w);
}

vec4 texture_blend() 
{
  if(layer_count == 0)
    return vec4 (1.0, 1.0, 1.0, 1.0);

  vec3 alpha = texture2D (alphamap, vary_alphacoord).rgb;
  float a0 = alpha.r;  
  float a1 = alpha.g;
  float a2 = alpha.b;

  vec3 t0 = texture2D(tex0, vary_texcoord).rgb;
  vec3 t1 = texture2D(tex1, vary_texcoord).rgb;
  vec3 t2 = texture2D(tex2, vary_texcoord).rgb;
  vec3 t3 = texture2D(tex3, vary_texcoord).rgb;

  return vec4 (t0 * (1.0 - (a0 + a1 + a2)) + t1 * a0 + t2 * a1 + t3 * a2, 1);
}

void main()
{
  vec3 fw = fwidth(vary_position.xyz);

  gl_FragColor = texture_blend();

  if(has_mccv)
  {
    gl_FragColor = vec4(gl_FragColor.rgb * vary_mccv, 1);
  }

  if(cant_paint)
  {
    gl_FragColor *= vec4(1, 0, 0, 1);
  }

  if(draw_areaid_overlay)
  {
    gl_FragColor = gl_FragColor * 0.3 + areaid_color;
  }

  if(draw_impassible_flag)
  {
    gl_FragColor = blend_by_alpha (vec4 (1.0, 1.0, 1.0, 0.5), gl_FragColor);
  }
  
  float shadow_alpha = texture2D (shadow_map, vary_alphacoord).a;

  gl_FragColor = vec4 (gl_FragColor.rgb * (1.0 - shadow_alpha), 1.0);

  if (draw_terrain_height_contour)
  {
    float f  = abs(fract(vary_position.y*0.25) - 0.5);
    float df = abs(fw.y * 0.25);
    float alpha  = smoothstep(-1.5*df, 1.5*df, f);

    gl_FragColor = vec4(gl_FragColor.rgb * alpha, 1);
  }

  if(draw_lines)
  {
    float fx = abs(fract((vary_position.x + CHUNKSIZE/2) / CHUNKSIZE) - 0.5);
    float dfx = abs(fw.x / CHUNKSIZE);
    float fz = abs(fract((vary_position.z + CHUNKSIZE/2) / CHUNKSIZE) - 0.5);
    float dfz = abs(fw.z / CHUNKSIZE);
    float alpha = 1 - min(smoothstep(0, dfx, fx), smoothstep(0, dfz, fz));

    gl_FragColor = blend_by_alpha (vec4(0.8, 0, 0, alpha), gl_FragColor);
  }

  if (draw_cursor_circle)
  {
    float diff = length(vary_position.xz - cursor_position.xz);
    diff = min(abs(diff - outer_cursor_radius), abs(diff - outer_cursor_radius * inner_cursor_ratio));
    float alpha = 1 - smoothstep(0, length(fw.xz), diff);

    gl_FragColor = blend_by_alpha (vec4(cursor_color.rgb, alpha), gl_FragColor);
  }
}
)code"
      }
    };

    opengl::scoped::use_program mcnk_shader{ mcnk_program };

    mcnk_shader.uniform("model_view", opengl::matrix::model_view());
    mcnk_shader.uniform("projection", opengl::matrix::projection());
    mcnk_shader.attrib("texcoord", detailtexcoords, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    mcnk_shader.attrib("alphacoord", alphatexcoords, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    mcnk_shader.uniform ("draw_lines", (int)draw_lines);
    if (cursor_type == 4)
    {
      mcnk_shader.uniform ("draw_cursor_circle", 1);
      mcnk_shader.uniform ("cursor_position", cursor_pos);
      mcnk_shader.uniform ("outer_cursor_radius", brushRadius);
      mcnk_shader.uniform ("inner_cursor_ratio", innerRadius);
      mcnk_shader.uniform ("cursor_color", cursor_color);
    }
    else
    {
      mcnk_shader.uniform ("draw_cursor_circle", 0);
    }   

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->draw ( frustum
                 , mcnk_shader
                 , culldistance
                 , camera_pos
                 , show_unpaintable_chunks
                 , draw_contour
                 , draw_paintability_overlay
                 , draw_chunk_flag_overlay
                 , draw_areaid_overlay
                 , draw_wireframe
                 , cursor_type
                 , area_id_colors
                 , mCurrentSelection
                 , animtime
                 );
    }

    for (int i = 0; i < 5; ++i)
    {
      opengl::texture::disable_texture(i);
    }
  }

  // Selection circle
  if (this->IsSelection(eEntry_MapChunk))
  {
    gl.polygonMode(GL_FRONT_AND_BACK, GL_LINE);

    gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);
    gl.disable(GL_CULL_FACE);
    //gl.depthMask(false);
    //gl.disable(GL_DEPTH_TEST);

    if (terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Quadra)
    {
      render_square(cursor_pos, brushRadius / 2.0f, 0.0f, brushRadius / 2.0f * innerRadius, true);
    }
    else
    {
      if (cursor_type == 1)
      {
        render_disk(cursor_pos, brushRadius, cursor_color);
        if (innerRadius >= 0.01f)
        {
          render_disk(cursor_pos, brushRadius * innerRadius, cursor_color, true);
        }
      }
      else if (cursor_type == 2)
      {
        render_sphere(cursor_pos, brushRadius, cursor_color);
      }
    }
    if (angled_mode && !use_ref_pos)
    {
      math::degrees o = math::degrees(orientation);
      float x = brushRadius * cos(o);
      float z = brushRadius * sin(o);
      float h = brushRadius * tan(math::degrees(angle));
      math::vector_3d const dest1 = cursor_pos + math::vector_3d(x, 0.f, z);
      math::vector_3d const dest2 = cursor_pos + math::vector_3d(x, h, z);
      render_line(cursor_pos, dest1);
      render_line(cursor_pos, dest2);
      render_line(dest1, dest2);
    }

    if (use_ref_pos)
    {
      render_sphere(ref_pos, 1.0f, cursor_color);

      math::vector_3d pos = cursor_pos;

      if (angled_mode)
      {
        // orient + 90.0f because of the rotation done in render_disk
        math::degrees a(angle), o(orientation+90.0f);
        pos.y = misc::angledHeight(ref_pos, pos, a, math::degrees(orientation));
        render_disk(pos, brushRadius, cursor_color, false, a, o);
        render_line(ref_pos, cursor_pos);
        render_line(ref_pos, pos);
      }
      else
      {
        pos.y = ref_pos.y;
        render_disk(pos, brushRadius, cursor_color);
      }

      render_line(cursor_pos, pos);
    }


    gl.enable(GL_CULL_FACE);
    gl.enable(GL_DEPTH_TEST);
    //GlDepthMask(true);
    gl.polygonMode(GL_FRONT_AND_BACK, GL_FILL);

  }

  if ( terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Vertex )
  {
    float size = (vertexCenter() - camera_pos).length();
    gl.pointSize(std::max(0.001f, 10.0f - (1.25f * size / CHUNKSIZE)));
    gl.color3f(1.0f, 0.0f, 0.0f);
    gl.begin(GL_POINTS);

    for (math::vector_3d const* pos : _vertices_selected)
    {
      gl.vertex3f(pos->x, pos->y + 0.1f, pos->z);
    }
    gl.end();

    gl.color3f(0.0f, 0.0f, 1.0f);
    render_sphere(vertexCenter(), 2.0f, cursor_color);
    gl.color3f(1.0f, 1.0f, 1.0f);
  }

  if (draw_mfbo)
  {
    //! \todo don't compile on every frame
    opengl::program const program
      { { GL_VERTEX_SHADER
        , R"code(
#version 110

attribute vec4 position;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * model_view * position;
}
)code"
        }
      , { GL_FRAGMENT_SHADER
        , R"code(
#version 110

uniform vec4 color;

void main()
{
  gl_FragColor = color;
}
)code"
        }
      };
    opengl::scoped::use_program mfbo_shader {program};

    mfbo_shader.uniform ("model_view", opengl::matrix::model_view());
    mfbo_shader.uniform ("projection", opengl::matrix::projection());

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawMFBO (mfbo_shader);
    }
  }

  opengl::texture::set_active_texture (1);
  opengl::texture::disable_texture();
  opengl::texture::set_active_texture (0);
  opengl::texture::enable_texture();

  gl.color4f(1, 1, 1, 1);
  gl.enable(GL_BLEND);

  gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, math::vector_4d (0.0f, 0.0f, 0.0f, 1.0f));
  gl.materiali(GL_FRONT_AND_BACK, GL_SHININESS, 0);

  gl.enable(GL_CULL_FACE);

  gl.disable(GL_BLEND);
  gl.disable(GL_ALPHA_TEST);

  // TEMP: for fucking around with lighting
  for (opengl::light light = GL_LIGHT0; light < GL_LIGHT0 + 8; ++light)
  {
    const float l_const( 0.0f );
    const float l_linear( 0.7f );
    const float l_quadratic( 0.03f );

    gl.lightf(light, GL_CONSTANT_ATTENUATION, l_const);
    gl.lightf(light, GL_LINEAR_ATTENUATION, l_linear);
    gl.lightf(light, GL_QUADRATIC_ATTENUATION, l_quadratic);
  }


  // M2s / models
  if (draw_models)
  {
    if (draw_model_animations)
      ModelManager::resetAnim();

    gl.enable(GL_LIGHTING);  //! \todo  Is this needed? Or does this fuck something up?
    for (std::map<int, ModelInstance>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it)
    {
      bool const is_hidden (hidden_models.count (it->second.model.get()));
      if (!is_hidden)
      {
        it->second.draw ( frustum
                        , culldistance
                        , camera_pos
                        , is_hidden
                        , draw_models_with_box
                        , draw_fog
                        , IsSelection (eEntry_Model) && boost::get<selected_model_type> (*GetCurrentSelection())->uid == it->second.uid
                        , animtime
                        );
      }
    }
  }




  // WMOs / map objects
  if (draw_wmo || mapIndex.hasAGlobalWMO())
  {
    gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, math::vector_4d (1.0f, 1.0f, 1.0f, 1.0f));
    gl.materiali(GL_FRONT_AND_BACK, GL_SHININESS, 10);

    gl.lightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    for (std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it)
    {
      bool const is_hidden (hidden_map_objects.count (it->second.wmo.get()));
      if (!is_hidden)
      {
        it->second.draw ( frustum
                        , culldistance
                        , camera_pos
                        , is_hidden
                        , draw_wmo_doodads
                        , draw_fog
                        , skies->colorSet[RIVER_COLOR_LIGHT]
                        , skies->colorSet[RIVER_COLOR_DARK]
                        , mCurrentSelection
                        , animtime
                        , [this] (bool on) { return outdoorLights (on); }
                        , skies->hasSkies()
                        , [this] (bool on) { return setupFog (on); }
                        );
      }
    }

    gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, math::vector_4d (0.0f, 0.0f, 0.0f, 1.0f));
    gl.materiali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
  }

  outdoorLights(true);
  setupFog (draw_fog);

  gl.color4f(1, 1, 1, 1);
  gl.enable(GL_BLEND);

  if (draw_water)
  {
    liquid_render liquid_renderer;

    opengl::scoped::use_program water_shader {liquid_renderer.shader_program()};

    water_shader.uniform ("model_view", opengl::matrix::model_view());
    water_shader.uniform ("projection", opengl::matrix::projection());

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawWater ( water_shader
                      , skies->colorSet[OCEAN_COLOR_LIGHT]
                      , skies->colorSet[OCEAN_COLOR_DARK]
                      , animtime
                      , water_layer
                      );
    }
  }
}

selection_result World::intersect ( math::ray const& ray
                                  , bool pOnlyMap
                                  , bool do_objects
                                  , bool draw_terrain
                                  , bool draw_wmo
                                  , bool draw_models
                                  , std::unordered_set<WMO*> const& hidden_map_objects
                                  , std::unordered_set<Model*> const& hidden_models
                                  )
{
  selection_result results;

  if (draw_terrain)
  {
    for (auto&& tile : mapIndex.loaded_tiles())
    {
      tile->intersect (ray, &results);
    }
  }

  if (!pOnlyMap && do_objects)
  {
    if (draw_models)
    {
      for (auto&& model_instance : mModelInstances)
      {
        bool const is_hidden (hidden_models.count (model_instance.second.model.get()));
        if (!is_hidden)
        {
          model_instance.second.intersect (ray, &results, animtime);
        }
      }
    }

    if (draw_wmo)
    {
      for (auto&& wmo_instance : mWMOInstances)
      {
        bool const is_hidden (hidden_map_objects.count (wmo_instance.second.wmo.get()));
        if (!is_hidden)
        {
          wmo_instance.second.intersect (ray, &results);
        }
      }
    }
  }

  return results;
}

void World::tick(float dt)
{
  while (dt > 0.1f) {
    ModelManager::updateEmitters(0.1f);
    dt -= 0.1f;
  }
  ModelManager::updateEmitters(dt);
}

unsigned int World::getAreaID (math::vector_3d const& pos)
{
  return for_maybe_chunk_at (pos, [&] (MapChunk* chunk) { return chunk->getAreaID(); }).get_value_or (-1);
}

void World::clearHeight(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk) {
    chunk->clearHeight();
  });
  for_all_chunks_on_tile(pos, [this] (MapChunk* chunk) {
      recalc_norms (chunk);
  });
}

void World::clearAllModelsOnADT(tile_index const& tile)
{
  std::vector<int> wmo_to_delete, m2_to_delete;

  for (auto it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it)
  {
    if (tile_index(it->second.pos) == tile)
    {
      wmo_to_delete.push_back(it->second.mUniqueID);
    }
  }

  for (auto it = mModelInstances.begin(); it != mModelInstances.end(); ++it)
  {
    if (tile_index(it->second.pos) == tile)
    {
      m2_to_delete.push_back(it->second.uid);
    }
  }

  for (int uid : wmo_to_delete)
  {
    deleteWMOInstance(uid);
  }
  for (int uid : m2_to_delete)
  {
    deleteModelInstance(uid);
  }
}

void World::CropWaterADT(const tile_index& pos)
{
  for_tile_at(pos, [](MapTile* tile) { tile->CropWater(); });
}

void World::setAreaID(math::vector_3d const& pos, int id, bool adt)
{
  if (adt)
  {
    for_all_chunks_on_tile(pos, [&](MapChunk* chunk) { chunk->setAreaID(id);});
  }
  else
  {
    for_chunk_at(pos, [&](MapChunk* chunk) { chunk->setAreaID(id);});
  }
}

void World::drawTileMode ( float /*ah*/
                         , math::vector_3d const& camera_pos
                         , bool draw_lines
                         , float zoom
                         , float aspect_ratio
                         )
{
  gl.clear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  gl.enable(GL_BLEND);

  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  opengl::scoped::matrix_pusher const matrix_outer;
  gl.scalef(zoom, zoom, 1.0f);

  {
    opengl::scoped::matrix_pusher const matrix;
    gl.translatef(-camera_pos.x / CHUNKSIZE, -camera_pos.z / CHUNKSIZE, 0);

    float minX = camera_pos.x / CHUNKSIZE - 2.0f * aspect_ratio / zoom;
    float maxX = camera_pos.x / CHUNKSIZE + 2.0f * aspect_ratio / zoom;
    float minY = camera_pos.z / CHUNKSIZE - 2.0f / zoom;
    float maxY = camera_pos.z / CHUNKSIZE + 2.0f / zoom;

    gl.enableClientState(GL_COLOR_ARRAY);
    gl.disableClientState(GL_NORMAL_ARRAY);
    gl.disableClientState(GL_TEXTURE_COORD_ARRAY);
    gl.disable(GL_CULL_FACE);
    gl.depthMask(GL_FALSE);

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawTextures (minX, minY, maxX, maxY, animtime);
    }

    gl.disableClientState(GL_COLOR_ARRAY);

    gl.enableClientState(GL_NORMAL_ARRAY);
    gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  if (draw_lines) {
    gl.translatef((GLfloat)fmod(-camera_pos.x / CHUNKSIZE, 16), (GLfloat)fmod(-camera_pos.z / CHUNKSIZE, 16), 0);
    /*  for(int x=-32;x<=48;x++)
    {
    if(x%16==0)
    gl.color4f(0.0f,1.0f,0.0f,0.5f);
    else
    gl.color4f(1.0f,0.0f,0.0f,0.5f);
    gl.begin(GL_LINES);
    gl.vertex3f(-32.0f,(float)x,-1);
    gl.vertex3f(48.0f,(float)x,-1);
    gl.vertex3f((float)x,-32.0f,-1);
    gl.vertex3f((float)x,48.0f,-1);
    gl.end();
    }*/

    for (float x = -32.0f; x <= 48.0f; x += 1.0f)
    {
      if (static_cast<int>(x) % 16)
        gl.color4f(1.0f, 0.0f, 0.0f, 0.5f);
      else
        gl.color4f(0.0f, 1.0f, 0.0f, 0.5f);
      gl.begin(GL_LINES);
      gl.vertex3f(-32.0f, x, -1);
      gl.vertex3f(48.0f, x, -1);
      gl.vertex3f(x, -32.0f, -1);
      gl.vertex3f(x, 48.0f, -1);
      gl.end();
    }
  }
}

bool World::GetVertex(float x, float z, math::vector_3d *V) const
{
  tile_index tile({x, 0, z});

  if (!mapIndex.tileLoaded(tile))
  {
    return false;
  }

  return mapIndex.getTile(tile)->GetVertex(x, z, V);
}

template<typename Fun>
  bool World::for_all_chunks_in_range (math::vector_3d const& pos, float radius, Fun&& fun)
{
  bool changed (false);

  for (MapTile* tile : mapIndex.tiles_in_range (pos, radius))
  {
    for (MapChunk* chunk : tile->chunks_in_range (pos, radius))
    {
      if (fun (chunk))
      {
        changed = true;
        mapIndex.setChanged (tile);
      }
    }
  }

  return changed;
}
template<typename Fun, typename Post>
  bool World::for_all_chunks_in_range (math::vector_3d const& pos, float radius, Fun&& fun, Post&& post)
{
  std::forward_list<MapChunk*> modified_chunks;

  bool changed ( for_all_chunks_in_range
                   ( pos, radius
                   , [&] (MapChunk* chunk)
                     {
                       if (fun (chunk))
                       {
                         modified_chunks.emplace_front (chunk);
                         return true;
                       }
                       return false;
                     }
                   )
               );

  for (MapChunk* chunk : modified_chunks)
  {
    post (chunk);
  }

  return changed;
}


void World::changeShader(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->ChangeMCCV(pos, color, change, radius, editMode);
      }
    );
}

void World::changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->changeTerrain(pos, change, radius, BrushType, inner_radius);
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, int flattenType, const math::vector_3d& origin, math::degrees angle, math::degrees orientation)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->flattenTerrain(pos, remain, radius, BrushType, flattenType, origin, angle, orientation);
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::blurTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->blurTerrain ( pos
                                  , remain
                                  , radius
                                  , BrushType
                                  , [this] (float x, float z) -> boost::optional<float>
                                    {
                                      math::vector_3d vec;
                                      auto res (GetVertex (x, z, &vec));
                                      return boost::make_optional (res, vec.y);
                                    }
                                  );
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::recalc_norms (MapChunk* chunk) const
{
  chunk->recalcNorms ( [this] (float x, float z) -> boost::optional<float>
                       {
                         math::vector_3d vec;
                         auto res (GetVertex (x, z, &vec));
                         return boost::make_optional (res, vec.y);
                       }
                     );
}

bool World::paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  return for_all_chunks_in_range
    ( pos, brush->getRadius()
    , [&] (MapChunk* chunk)
      {
        return chunk->paintTexture(pos, brush, strength, pressure, texture);
      }
    );
}

bool World::sprayTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture)
{
  bool succ = false;
  float inc = brush->getRadius() / 4.0f;

  for (float pz = pos.z - spraySize; pz < pos.z + spraySize; pz += inc)
  {
    for (float px = pos.x - spraySize; px < pos.x + spraySize; px += inc)
    {
      if ((sqrt(pow(px - pos.x, 2) + pow(pz - pos.z, 2)) <= spraySize) && ((rand() % 1000) < sprayPressure))
      {
        succ |= paintTexture({px, pos.y, pz}, brush, strength, pressure, texture);
      }
    }
  }

  return succ;
}

void World::eraseTextures(math::vector_3d const& pos)
{
  for_chunk_at(pos, [](MapChunk* chunk) {chunk->eraseTextures();});
}

void World::overwriteTextureAtCurrentChunk(math::vector_3d const& pos, scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture)
{
  for_chunk_at(pos, [&](MapChunk* chunk) {chunk->switchTexture(oldTexture, newTexture);});
}

void World::setHole(math::vector_3d const& pos, bool big, bool hole)
{
  for_chunk_at(pos, [&](MapChunk* chunk) { chunk->setHole(pos, big, hole); });
}

void World::setHoleADT(math::vector_3d const& pos, bool hole)
{
  for_all_chunks_on_tile(pos, [&](MapChunk* chunk) { chunk->setHole(pos, true, hole); });
}


template<typename Fun>
  void World::for_all_chunks_on_tile (math::vector_3d const& pos, Fun&& fun)
{
  MapTile* tile (mapIndex.getTile (pos));
  mapIndex.setChanged (tile);

  for (size_t ty = 0; ty < 16; ++ty)
  {
    for (size_t tx = 0; tx < 16; ++tx)
    {
      fun (tile->getChunk (ty, tx));
    }
  }
}

template<typename Fun>
  auto World::for_chunk_at(math::vector_3d const& pos, Fun&& fun) -> decltype (fun (nullptr))
  {
    MapTile* tile(mapIndex.getTile(pos));
    mapIndex.setChanged(tile);

    return fun(tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }

template<typename Fun>
  auto World::for_maybe_chunk_at(math::vector_3d const& pos, Fun&& fun) -> boost::optional<decltype (fun (nullptr))>
{
  MapTile* tile (mapIndex.getTile (pos));
  if (tile)
  {
    return fun (tile->getChunk ((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }
  else
  {
    return boost::none;
  }
}

template<typename Fun>
  void World::for_tile_at(tile_index const& pos, Fun&& fun)
  {
    MapTile* tile(mapIndex.getTile(pos));
    if (tile)
    {
      mapIndex.setChanged(tile);
      fun(tile);
    }
  }

void World::convert_alphamap(bool to_big_alpha)
{
  if (to_big_alpha == mapIndex.hasBigAlpha())
  {
    return;
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile);

      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->convert_alphamap(to_big_alpha);
        mTile->saveTile (false, this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }

  mapIndex.convert_alphamap(to_big_alpha);
  mapIndex.save();
}

void World::saveMap (int width, int height)
{
  //! \todo  Output as BLP.
  unsigned char image[256 * 256 * 3];
  MapTile *ATile;
  FILE *fid;
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.readBuffer(GL_BACK);

  float minX = -64 * 16;
  float maxX = 64 * 16;
  float minY = -64 * 16;
  float maxY = 64 * 16;

  gl.enableClientState(GL_COLOR_ARRAY);
  gl.disableClientState(GL_NORMAL_ARRAY);
  gl.disableClientState(GL_TEXTURE_COORD_ARRAY);

  for (int y = 0; y<64; y++)
  {
    for (int x = 0; x<64; x++)
    {
      tile_index tile(x, y);

      if (!mapIndex.hasTile(tile))
      {
        continue;
      }

      ATile = mapIndex.loadTile(tile);
      gl.clear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      opengl::scoped::matrix_pusher const matrix;
      gl.scalef(0.08333333f, 0.08333333f, 1.0f);

      //gl.translatef(-camera_pos.x/CHUNKSIZE,-camera_pos.z/CHUNKSIZE,0);
      gl.translatef(x * -16.0f - 8.0f, y * -16.0f - 8.0f, 0.0f);


      ATile->drawTextures (minX, minY, maxX, maxY, animtime);
      gl.readPixels(width / 2 - 128, height / 2 - 128, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, image);

      std::stringstream ss;
      ss << basename.c_str() << "_map_" << x << "_" << y << ".raw";
      fid = fopen(ss.str().c_str(), "wb");
      fwrite(image, 256 * 3, 256, fid);
      fclose(fid);
    }
  }

  gl.disableClientState(GL_COLOR_ARRAY);

  gl.enableClientState(GL_NORMAL_ARRAY);
  gl.enableClientState(GL_TEXTURE_COORD_ARRAY);
}

void World::deleteModelInstance(int pUniqueID)
{
  std::map<int, ModelInstance>::iterator it = mModelInstances.find(pUniqueID);
  if (it == mModelInstances.end()) return;

  updateTilesModel(&it->second);
  mModelInstances.erase(it);
  ResetSelection();
}

void World::deleteWMOInstance(int pUniqueID)
{
  std::map<int, WMOInstance>::iterator it = mWMOInstances.find(pUniqueID);
  if (it == mWMOInstances.end()) return;

  updateTilesWMO(&it->second);
  mWMOInstances.erase(it);
  ResetSelection();
}

void World::delete_duplicate_model_and_wmo_instances()
{
  std::unordered_set<int> wmos_to_remove;
  std::unordered_set<int> models_to_remove;

  for (auto lhs(mWMOInstances.begin()); lhs != mWMOInstances.end(); ++lhs)
  {
    for (auto rhs(std::next(lhs)); rhs != mWMOInstances.end(); ++rhs)
    {
      assert(lhs->first != rhs->first);

      if ( lhs->second.pos == rhs->second.pos
        && lhs->second.dir == rhs->second.dir
        && lhs->second.wmo->_filename == rhs->second.wmo->_filename
         )
      {
        wmos_to_remove.emplace(rhs->second.mUniqueID);
      }
    }
  }

  for (auto lhs(mModelInstances.begin()); lhs != mModelInstances.end(); ++lhs)
  {
    for (auto rhs(std::next(lhs)); rhs != mModelInstances.end(); ++rhs)
    {
      assert(lhs->first != rhs->first);

      if ( lhs->second.pos == rhs->second.pos
        && lhs->second.dir == rhs->second.dir
        && lhs->second.scale == rhs->second.scale
        && lhs->second.model->_filename == rhs->second.model->_filename
        )
      {
        models_to_remove.emplace(rhs->second.uid);
      }
    }
  }

  for (int uid : wmos_to_remove)
  {
    deleteWMOInstance(uid);
  }
  for (int uid : models_to_remove)
  {
    deleteModelInstance(uid);
  }

  Log << "Deleted " << wmos_to_remove.size() << " duplicate WMOs" << std::endl;
  Log << "Deleted " << models_to_remove.size() << " duplicate models" << std::endl;
}

void World::addM2 ( std::string const& filename
                  , math::vector_3d newPos
                  , float scale
                  , math::vector_3d rotation
                  , noggit::object_paste_params* paste_params
                  )
{
  ModelInstance newModelis = ModelInstance(filename);

  newModelis.uid = mapIndex.newGUID();
  newModelis.pos = newPos;
  newModelis.scale = scale;
  newModelis.dir = rotation;

  if (Settings::getInstance()->random_rotation)
  {
    float min = paste_params->minRotation;
    float max = paste_params->maxRotation;
    newModelis.dir.y += misc::randfloat(min, max);
  }

  if (Settings::getInstance()->random_tilt)
  {
    float min = paste_params->minTilt;
    float max = paste_params->maxTilt;
    newModelis.dir.x += misc::randfloat(min, max);
    newModelis.dir.z += misc::randfloat(min, max);
  }

  if (Settings::getInstance()->random_size)
  {
    float min = paste_params->minScale;
    float max = paste_params->maxScale;
    newModelis.scale = misc::randfloat(min, max);
  }

  newModelis.recalcExtents();
  updateTilesModel(&newModelis);
  mModelInstances.emplace(newModelis.uid, newModelis);
}

void World::addWMO ( std::string const& filename
                   , math::vector_3d newPos
                   , math::vector_3d rotation
                   )
{
  WMOInstance newWMOis(filename);

  newWMOis.mUniqueID = mapIndex.newGUID();
  newWMOis.pos = newPos;
  newWMOis.dir = rotation;

  // recalc the extends
  newWMOis.recalcExtents();
  updateTilesWMO(&newWMOis);

  mWMOInstances.emplace(newWMOis.mUniqueID, newWMOis);
}

void World::reload_tile(tile_index const& tile)
{
  ResetSelection();
  // to remove the new models and reload the old ones
  clearAllModelsOnADT(tile);
  mapIndex.reloadTile(tile);
}

void World::updateTilesEntry(selection_type const& entry)
{
  if (entry.which() == eEntry_WMO)
  {
    updateTilesWMO (boost::get<selected_wmo_type> (entry));
  }
  else if (entry.which() == eEntry_Model)
  {
    updateTilesModel (boost::get<selected_model_type> (entry));
  }
}

void World::updateTilesWMO(WMOInstance* wmo)
{
  tile_index start(wmo->extents[0]), end(wmo->extents[1]);
  for (int z = start.z; z <= end.z; ++z)
  {
    for (int x = start.x; x <= end.x; ++x)
    {
      mapIndex.setChanged(tile_index(x, z));
    }
  }
}

void World::updateTilesModel(ModelInstance* m2)
{
  tile_index start(m2->extents[0]), end(m2->extents[1]);
  for (int z = start.z; z <= end.z; ++z)
  {
    for (int x = start.x; x <= end.x; ++x)
    {
      mapIndex.setChanged(tile_index(x, z));
    }
  }
}

unsigned int World::getMapID()
{
  return mapIndex._map_id;
}

void World::clearTextures(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    chunk->eraseTextures();
  });  
}

void World::setBaseTexture(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    chunk->eraseTextures();
    if (!!noggit::ui::selected_texture::get())
    {
      chunk->addTexture(*noggit::ui::selected_texture::get());
    }
  });
}

void World::swapTexture(math::vector_3d const& pos, scoped_blp_texture_reference tex)
{
  if (!!noggit::ui::selected_texture::get())
  {
    for_all_chunks_on_tile(pos, [&](MapChunk* chunk) { chunk->switchTexture(tex, *noggit::ui::selected_texture::get()); });
  }
}

void World::removeTexDuplicateOnADT(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk) { chunk->_texture_set.removeDuplicate(); } );
}

void World::change_texture_flag(math::vector_3d const& pos, scoped_blp_texture_reference tex, std::size_t flag, bool add)
{
  for_chunk_at(pos, [&] (MapChunk* chunk) { chunk->change_texture_flag(tex, flag, add); });
}

void World::paintLiquid( math::vector_3d const& pos
                       , float radius
                       , int liquid_id
                       , bool add
                       , math::radians const& angle
                       , math::radians const& orientation
                       , bool lock
                       , math::vector_3d const& origin
                       , bool override_height
                       , bool override_liquid_id
                       , float opacity_factor
                       )
{
  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk)
  {
    chunk->liquid_chunk()->paintLiquid(pos, radius, liquid_id, add, angle, orientation, lock, origin, override_height, override_liquid_id, chunk, opacity_factor);
    return true;
  });
}

bool World::canWaterSave(const tile_index& tile)
{
  MapTile* mt = mapIndex.getTile(tile);
  return !!mt && mt->canWaterSave();
}

void World::setWaterType(const tile_index& pos, int type, int layer)
{
  for_tile_at ( pos
              , [&] (MapTile* tile)
                {
                  tile->Water.setType (type, layer);
                }
              );
}

int World::getWaterType(const tile_index& tile, int layer)
{
  if (mapIndex.tileLoaded(tile))
  {
    return mapIndex.getTile(tile)->Water.getType (layer);
  }
  else
  {
    return 0;
  }
}

void World::autoGenWaterTrans(const tile_index& pos, float factor)
{
  for_tile_at(pos, [&](MapTile* tile) { tile->Water.autoGen(factor); });
}


void World::fixAllGaps()
{
  std::vector<MapChunk*> chunks;

  for (MapTile* tile : mapIndex.loaded_tiles())
  {
    MapTile* left = mapIndex.getTileLeft(tile);
    MapTile* above = mapIndex.getTileAbove(tile);
    bool tileChanged = false;

    // fix the gaps with the adt at the left of the current one
    if (left)
    {
      for (size_t ty = 0; ty < 16; ty++)
      {
        MapChunk* chunk = tile->getChunk(0, ty);
        if (chunk->fixGapLeft(left->getChunk(15, ty)))
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }

    // fix the gaps with the adt above the current one
    if (above)
    {
      for (size_t tx = 0; tx < 16; tx++)
      {
        MapChunk* chunk = tile->getChunk(tx, 0);
        if (chunk->fixGapAbove(above->getChunk(tx, 15)))
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }

    // fix gaps within the adt
    for (size_t ty = 0; ty < 16; ty++)
    {
      for (size_t tx = 0; tx < 16; tx++)
      {
        MapChunk* chunk = tile->getChunk(tx, ty);
        bool changed = false;

        // if the chunk isn't the first of the row
        if (tx && chunk->fixGapLeft(tile->getChunk(tx - 1, ty)))
        {
          changed = true;
        }

        // if the chunk isn't the first of the column
        if (ty && chunk->fixGapAbove(tile->getChunk(tx, ty - 1)))
        {
          changed = true;
        }

        if (changed)
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }
    if (tileChanged)
    {
      mapIndex.setChanged(tile);
    }
  }

  for (MapChunk* chunk : chunks)
  {
    recalc_norms (chunk);
  }
}

bool World::isUnderMap(math::vector_3d const& pos)
{
  tile_index const tile (pos);

  if (mapIndex.tileLoaded(tile))
  {
    size_t chnkX = (pos.x / CHUNKSIZE) - tile.x * 16;
    size_t chnkZ = (pos.z / CHUNKSIZE) - tile.z * 16;

    // check using the cursor height
    return (mapIndex.getTile(tile)->getChunk(chnkX, chnkZ)->getMinHeight()) > pos.y + 2.0f;
  }

  return true;
}

void World::selectVertices(math::vector_3d const& pos, float radius)
{
  _vertex_center_updated = false;
  _vertex_border_updated = false;

  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk){
    _vertex_chunks.emplace(chunk);
    _vertex_tiles.emplace(chunk->mt);
    chunk->selectVertex(pos, radius, _vertices_selected);
    return true;
  });
}

bool World::deselectVertices(math::vector_3d const& pos, float radius)
{
  _vertex_center_updated = false;
  _vertex_border_updated = false;
  std::set<math::vector_3d*> inRange;

  for (math::vector_3d* v : _vertices_selected)
  {
    if (misc::dist(*v, pos) <= radius)
    {
      inRange.emplace(v);
    }
  }

  for (math::vector_3d* v : inRange)
  {
    _vertices_selected.erase(v);
  }

  return _vertices_selected.empty();
}

void World::moveVertices(float h)
{
  _vertex_center_updated = false;
  for (math::vector_3d* v : _vertices_selected)
  {
    v->y += h;
  }

  updateVertexCenter();
  updateSelectedVertices();
}

void World::updateSelectedVertices()
{
  for (MapTile* tile : _vertex_tiles)
  {
    mapIndex.setChanged(tile);
  }

  // fix only the border chunks to be more efficient
  for (MapChunk* chunk : vertexBorderChunks())
  {
    chunk->fixVertices(_vertices_selected);
  }

  for (MapChunk* chunk : _vertex_chunks)
  {
    chunk->updateVerticesData();
    recalc_norms (chunk);
  }
}

void World::orientVertices ( math::vector_3d const& ref_pos
                           , math::degrees vertex_angle
                           , math::degrees vertex_orientation
                           )
{
  for (math::vector_3d* v : _vertices_selected)
  {
    v->y = misc::angledHeight(ref_pos, *v, vertex_angle, vertex_orientation);
  }
  updateSelectedVertices();
}

void World::flattenVertices (float height)
{
  for (math::vector_3d* v : _vertices_selected)
  {
    v->y = height;
  }
  updateSelectedVertices();
}

void World::clearVertexSelection()
{
  _vertex_border_updated = false;
  _vertex_center_updated = false;
  _vertices_selected.clear();
  _vertex_chunks.clear();
  _vertex_tiles.clear();
}

void World::updateVertexCenter()
{
  _vertex_center_updated = true;
  _vertex_center = { 0,0,0 };
  float f = 1.0f / _vertices_selected.size();
  for (math::vector_3d* v : _vertices_selected)
  {
    _vertex_center += (*v) * f;
  }
}

math::vector_3d const& World::vertexCenter()
{
  if (!_vertex_center_updated)
  {
    updateVertexCenter();
  }

  return _vertex_center;
}

std::set<MapChunk*>& World::vertexBorderChunks()
{
  if (!_vertex_border_updated)
  {
    _vertex_border_updated = true;
    _vertex_border_chunks.clear();

    for (MapChunk* chunk : _vertex_chunks)
    {
      if (chunk->isBorderChunk(_vertices_selected))
      {
        _vertex_border_chunks.emplace(chunk);
      }
    }
  }
  return _vertex_border_chunks;
}
