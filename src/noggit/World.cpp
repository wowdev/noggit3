// This file is part of Noggit3, licensed under GNU General Public License (version 3).


#include <noggit/World.h>

#include <math/frustum.hpp>
#include <noggit/Brush.h> // brush
#include <noggit/ChunkWater.hpp>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h>
#include <noggit/TileWater.hpp>// tile water
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/map_index.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/TexturingGUI.h>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/thread.hpp>

#include <QtWidgets/QMessageBox>

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
  , _settings (new QSettings())
  , _view_distance(_settings->value ("view_distance", 1000.f).toFloat())
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
  if (draw_fog) 
  {
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
  else 
  {
    gl.disable(GL_FOG);
    culldistance = _view_distance;
  }
}

void World::draw ( math::matrix_4x4 const& model_view
                 , math::matrix_4x4 const& projection
                 , math::vector_3d const& cursor_pos
                 , math::vector_4d const& cursor_color
                 , int cursor_type
                 , float brush_radius
                 , bool show_unpaintable_chunks
                 , bool draw_contour
                 , float inner_radius_ratio
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
                 , bool draw_hidden_models
                 , std::map<int, misc::random_color>& area_id_colors
                 , bool draw_fog
                 , eTerrainType ground_editing_brush
                 , int water_layer
                 , display_mode display
                 )
{
  if (!_display_initialized)
  {
    initDisplay();
    _display_initialized = true;
  }

  math::matrix_4x4 const mvp(model_view * projection);
  math::frustum const frustum (mvp);

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
  gl.disable(GL_DEPTH_TEST);

  int daytime = static_cast<int>(time) % 2880;
  outdoorLightStats = ol->getLightStats(daytime);
  skies->update_sky_colors(camera_pos, daytime);

  if (!hadSky)
  {
    hadSky = skies->draw ( mvp
                         , camera_pos
                         , outdoorLightStats.nightIntensity
                         , draw_fog
                         , animtime
                         );
  }
  // clearing the depth buffer only - color buffer is/has been overwritten anyway
  // unless there is no sky OR skybox
  GLbitfield clearmask = GL_DEPTH_BUFFER_BIT;
  if (!hadSky)
  {
    clearmask |= GL_COLOR_BUFFER_BIT;
  }
  gl.clear(clearmask);


  outdoorLighting();
  outdoorLights(true);

  gl.fogi(GL_FOG_MODE, GL_LINEAR);
  setupFog (draw_fog);

  // Draw verylowres heightmap
  if (draw_fog && draw_terrain) 
  {
    _horizon_render->draw (model_view, projection, &mapIndex, skies->color_set[FOG_COLOR], culldistance, frustum, camera_pos, display);
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
    if (!_mcnk_program)
    {
      _mcnk_program.reset(new opengl::program({ { GL_VERTEX_SHADER
        , R"code(
#version 330 core

in vec3 position;
in vec3 normal;
in vec3 mccv;
in vec2 texcoord;

uniform mat4 model_view;
uniform mat4 projection;

out vec3 vary_position;
out vec2 vary_texcoord;
out vec3 vary_normal;
out vec3 vary_mccv;

void main()
{
  gl_Position = projection * model_view * vec4(position, 1.0);
  vary_normal = normal;
  vary_position = position;
  vary_texcoord = texcoord;
  vary_mccv = mccv;
}
)code"
        }
        ,{ GL_FRAGMENT_SHADER
        , R"code(
#version 330 core

uniform mat4 model_view;

uniform sampler2D shadow_map;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform vec2 tex_anim_0;
uniform vec2 tex_anim_1;
uniform vec2 tex_anim_2;
uniform vec2 tex_anim_3;
uniform sampler2D alphamap;
uniform int layer_count;
uniform bool has_mccv;
uniform bool cant_paint;
uniform bool draw_areaid_overlay;
uniform vec4 areaid_color;
uniform bool draw_impassible_flag;
uniform bool draw_terrain_height_contour;
uniform bool draw_lines;
uniform bool draw_hole_lines;

uniform bool draw_wireframe;
uniform int wireframe_type;
uniform float wireframe_radius;
uniform float wireframe_width;
uniform vec4 wireframe_color;
uniform bool rainbow_wireframe;

uniform vec3 camera;
uniform bool draw_fog;
uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;

uniform bool draw_cursor_circle;
uniform vec3 cursor_position;
uniform float outer_cursor_radius;
uniform float inner_cursor_ratio;
uniform vec4 cursor_color;

uniform vec3 light_dir;
uniform vec3 diffuse_color;
uniform vec3 ambient_color;

in vec3 vary_position;
in vec2 vary_texcoord;
in vec3 vary_normal;
in vec3 vary_mccv;

out vec4 out_color;

const float TILESIZE  = 533.33333;
const float CHUNKSIZE = TILESIZE / 16.0;
const float HOLESIZE  = CHUNKSIZE * 0.25;
const float UNITSIZE = HOLESIZE * 0.5;

vec4 blend_by_alpha (in vec4 source, in vec4 dest)
{
  return source * source.w + dest * (1.0 - source.w);
}

vec4 texture_blend() 
{
  if(layer_count == 0)
    return vec4 (1.0, 1.0, 1.0, 1.0);

  vec3 alpha = texture2D (alphamap, vary_texcoord / 8.0).rgb;
  float a0 = alpha.r;  
  float a1 = alpha.g;
  float a2 = alpha.b;

  vec3 t0 = texture2D(tex0, vary_texcoord + tex_anim_0).rgb;
  vec3 t1 = texture2D(tex1, vary_texcoord + tex_anim_1).rgb;
  vec3 t2 = texture2D(tex2, vary_texcoord + tex_anim_2).rgb;
  vec3 t3 = texture2D(tex3, vary_texcoord + tex_anim_3).rgb;

  return vec4 (t0 * (1.0 - (a0 + a1 + a2)) + t1 * a0 + t2 * a1 + t3 * a2, 1.0);
}

float contour_alpha(float unit_size, float pos, float line_width)
{
  float f = abs(fract((pos + unit_size*0.5) / unit_size) - 0.5);
  float df = abs(line_width / unit_size);
  return smoothstep(0.0, df, f);
}

float contour_alpha(float unit_size, vec2 pos, vec2 line_width)
{
  return 1.0 - min( contour_alpha(unit_size, pos.x, line_width.x)
                  , contour_alpha(unit_size, pos.y, line_width.y)
                  );
}

float dist_3d(vec3 a, vec3 b)
{
  float x = a.x - b.x;
  float y = a.y - b.y;
  float z = a.z - b.z;
  return sqrt(x*x + y*y + z*z);
}

void main()
{
  float dist_from_camera = dist_3d(camera, vary_position);

  if(draw_fog && dist_from_camera >= fog_end)
  {
    out_color = fog_color;
    return;
  } 
  vec3 fw = fwidth(vary_position.xyz);

  out_color = texture_blend();
  out_color.rgb *= vary_mccv;

  // diffuse + ambient lighting
  out_color.rgb *= vec3(clamp (diffuse_color * max(dot(vary_normal, light_dir), 0.0), 0.0, 1.0)) + ambient_color;

  if(cant_paint)
  {
    out_color *= vec4(1.0, 0.0, 0.0, 1.0);
  }
  
  if(draw_areaid_overlay)
  {
    out_color = out_color * 0.3 + areaid_color;
  }

  if(draw_impassible_flag)
  {
    out_color = blend_by_alpha (vec4 (1.0, 1.0, 1.0, 0.5), out_color);
  }

  float shadow_alpha = texture2D (shadow_map, vary_texcoord / 8.0).r;

  out_color = vec4 (out_color.rgb * (1.0 - shadow_alpha), 1.0);

  if (draw_terrain_height_contour)
  {
    out_color = vec4(out_color.rgb * contour_alpha(4.0, vary_position.y+0.1, fw.y), 1.0);
  }

  bool lines_drawn = false;
  if(draw_lines)
  {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    color.a = contour_alpha(TILESIZE, vary_position.xz, fw.xz * 1.5);
    color.g = color.a > 0.0 ? 0.8 : 0.0;

    if(color.a == 0.0)
    {
      color.a = contour_alpha(CHUNKSIZE, vary_position.xz, fw.xz);
      color.r = color.a > 0.0 ? 0.8 : 0.0;
    }
    if(draw_hole_lines && color.a == 0.0)
    {
      color.a = contour_alpha(HOLESIZE, vary_position.xz, fw.xz * 0.75);
      color.b = 0.8;
    }
    
    lines_drawn = color.a > 0.0;
    out_color = blend_by_alpha (color, out_color);
  }

  if(draw_fog && dist_from_camera >= fog_end * fog_start)
  {
    float start = fog_end * fog_start;
    float alpha = (dist_from_camera - start) / (fog_end - start);
    out_color = blend_by_alpha (vec4(fog_color.rgb, alpha), out_color);
    out_color.a = 1.0;
  }

  if(draw_wireframe && !lines_drawn)
  {
    // true by default => type 0
	  bool draw_wire = true;
    float real_wireframe_radius = max(outer_cursor_radius * wireframe_radius, 2.0 * UNITSIZE); 
	
	  if(wireframe_type == 1)
	  {
		  draw_wire = (length(vary_position.xz - cursor_position.xz) < real_wireframe_radius);
	  }
	
	  if(draw_wire)
	  {
		  float alpha = 0.0;

		  alpha = contour_alpha(UNITSIZE, vary_position.xz, fw.xz * wireframe_width);

		  float xmod = mod(vary_position.x, UNITSIZE);
		  float zmod = mod(vary_position.z, UNITSIZE);
		  float d = length(fw.xz) * wireframe_width;
		  float diff = min( min(abs(xmod - zmod), abs(xmod - UNITSIZE + zmod))
                      , min(abs(zmod - xmod), abs(zmod + UNITSIZE - zmod))
                      );        

		  alpha = max(alpha, 1.0 - smoothstep(0.0, d, diff));
      vec4 color;
 
      if(rainbow_wireframe)
      {
        float pct = (vary_position.x - cursor_position.x + real_wireframe_radius) / (2.0 * real_wireframe_radius);          
        float red = (1.0 - smoothstep(0.2, 0.4, pct)) + smoothstep(0.8, 1.0, pct);
        float green = (pct < 0.6 ? smoothstep(0.0, 0.2, pct) : (1.0 - smoothstep(0.6, 0.8, pct)));
        float blue = smoothstep(0.4, 0.6, pct);

        color = vec4(red, green, blue, alpha);
      }
      else
      {
        color = vec4(wireframe_color.rgb, alpha * wireframe_color.a);
      }      

		  out_color = blend_by_alpha (color, out_color);
	  }
  }

  if (draw_cursor_circle)
  {
    float diff = length(vary_position.xz - cursor_position.xz);
    diff = min(abs(diff - outer_cursor_radius), abs(diff - outer_cursor_radius * inner_cursor_ratio));
    float alpha = 1.0 - smoothstep(0.0, length(fw.xz), diff);

    out_color = blend_by_alpha (vec4(cursor_color.rgb, alpha), out_color);
  }
}
)code"
        }
      } ));
    }
    
    opengl::scoped::use_program mcnk_shader{ *_mcnk_program.get() };

    mcnk_shader.uniform("model_view", model_view);
    mcnk_shader.uniform("projection", projection);
    
    mcnk_shader.uniform ("draw_lines", (int)draw_lines);
    mcnk_shader.uniform ("draw_hole_lines", (int)draw_hole_lines);
    mcnk_shader.uniform("draw_areaid_overlay", (int)draw_areaid_overlay);
    mcnk_shader.uniform ("draw_terrain_height_contour", (int)draw_contour);

    // the flag stays on if the last chunk drawn before leaving the editing tool has it
    if (!draw_chunk_flag_overlay)
    {
      mcnk_shader.uniform ("draw_impassible_flag", 0);
    }

    mcnk_shader.uniform ("draw_wireframe", (int)draw_wireframe);
    mcnk_shader.uniform ("wireframe_type", _settings->value("wireframe/type", 0).toInt()); 
    mcnk_shader.uniform ("wireframe_radius", _settings->value("wireframe/radius", 1.5f).toFloat());
    mcnk_shader.uniform ("wireframe_width", _settings->value ("wireframe/width", 1.f).toFloat());   
    // !\ todo store the color somewhere ?
    QColor c = _settings->value("wireframe/color").value<QColor>();
    math::vector_4d wireframe_color(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    mcnk_shader.uniform ("wireframe_color", wireframe_color);
    mcnk_shader.uniform ("rainbow_wireframe", _settings->value("wireframe/rainbow", 0).toInt());
    
    mcnk_shader.uniform ("draw_fog", (int)draw_fog);
    mcnk_shader.uniform ("fog_color", math::vector_4d(skies->color_set[FOG_COLOR], 1));
    // !\ todo use light dbcs values
    mcnk_shader.uniform ("fog_end", fogdistance);
    mcnk_shader.uniform ("fog_start", 0.5f);
    mcnk_shader.uniform ("camera", camera_pos);

    
    math::vector_3d dd = outdoorLightStats.dayDir;
    math::vector_3d diffuse_color(skies->color_set[LIGHT_GLOBAL_DIFFUSE] * outdoorLightStats.dayIntensity);
    math::vector_3d ambient_color(skies->color_set[LIGHT_GLOBAL_AMBIENT] * outdoorLightStats.ambientIntensity);  

    mcnk_shader.uniform("light_dir", math::vector_3d(-dd.x, -dd.z, dd.y));
    mcnk_shader.uniform("diffuse_color", diffuse_color);
    mcnk_shader.uniform("ambient_color", ambient_color);
    

    if (cursor_type == 4)
    {
      mcnk_shader.uniform ("draw_cursor_circle", 1);
      mcnk_shader.uniform ("cursor_position", cursor_pos);
      mcnk_shader.uniform ("outer_cursor_radius", brush_radius);
      mcnk_shader.uniform ("inner_cursor_ratio", inner_radius_ratio);
      mcnk_shader.uniform ("cursor_color", cursor_color);
    }
    else
    {
      mcnk_shader.uniform ("draw_cursor_circle", 0);
    }

    mcnk_shader.uniform("alphamap", 0);
    mcnk_shader.uniform("tex0", 1);
    mcnk_shader.uniform("tex1", 2);
    mcnk_shader.uniform("tex2", 3);
    mcnk_shader.uniform("tex3", 4);
    mcnk_shader.uniform("shadow_map", 5);

    mcnk_shader.uniform("tex_anim_0", math::vector_2d());
    mcnk_shader.uniform("tex_anim_1", math::vector_2d());
    mcnk_shader.uniform("tex_anim_2", math::vector_2d());
    mcnk_shader.uniform("tex_anim_3", math::vector_2d());

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->draw ( frustum
                 , mcnk_shader
                 , detailtexcoords
                 , culldistance
                 , camera_pos
                 , show_unpaintable_chunks
                 , draw_paintability_overlay
                 , draw_chunk_flag_overlay
                 , draw_areaid_overlay
                 , area_id_colors
                 , mCurrentSelection
                 , animtime
                 , display
                 );
    }
  }
  
  // 4 = terrain shader cursor
  if(cursor_type != 4)
  {
    opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
    gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    noggit::cursor_render::mode mode;

    if (terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Quadra)
    {
      mode = cursor_type == 2 
        ? noggit::cursor_render::mode::square
        : noggit::cursor_render::mode::cube;
    }
    else if (cursor_type == 2)
    {
      mode = noggit::cursor_render::mode::sphere;
    }
    else
    {
      mode = noggit::cursor_render::mode::circle;
    }

    _cursor_render.draw(mode, mvp, cursor_color, cursor_pos, brush_radius, inner_radius_ratio);
  }
  
  if (use_ref_pos)
  {
    _sphere_render.draw(mvp, ref_pos, cursor_color, 2.f);    
  }  

  if (terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Vertex)
  {
    float size = (vertexCenter() - camera_pos).length();
    gl.pointSize(std::max(0.001f, 10.0f - (1.25f * size / CHUNKSIZE)));

    for (math::vector_3d const* pos : _vertices_selected)
    {
      _sphere_render.draw(mvp, *pos, math::vector_4d(1.f, 0.f, 0.f, 1.f), 0.5f);
    }

    _sphere_render.draw(mvp, vertexCenter(), cursor_color, 2.f);
  }

  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> cull;

    math::degrees orient = math::degrees(orientation);
    math::degrees incl = math::degrees(angle);
    math::vector_4d color = cursor_color;
    // always half transparent regardless or the cursor transparency
    color.w = 0.5f;

    float radius = 1.2f * brush_radius;

    if (angled_mode && !use_ref_pos)
    {   
      math::vector_3d pos = cursor_pos;
      pos.y += 0.1f; // to avoid z-fighting with the ground
      _square_render.draw(mvp, pos, radius, incl, orient, color);
    }
    else if (use_ref_pos)
    {
      if (angled_mode)
      {
        math::vector_3d pos = cursor_pos;
        pos.y = misc::angledHeight(ref_pos, pos, incl, orient);
        pos.y += 0.1f;
        _square_render.draw(mvp, pos, radius, incl, orient, color);

        // display the plane when the cursor is far from ref_point
        if(misc::dist(pos.x, pos.z, ref_pos.x, ref_pos.z) > 10.f + radius)
        {
          math::vector_3d ref = ref_pos;
          ref.y += 0.1f;
          _square_render.draw(mvp, ref, 10.f, incl, orient, color);
        }
      }
      else
      {
        math::vector_3d pos = cursor_pos;
        pos.y = ref_pos.y + 0.1f;
        _square_render.draw(mvp, pos, radius, math::degrees(0.f), math::degrees(0.f), color);
      }
    }
  }

  std::unordered_map<std::string, std::vector<ModelInstance*>> _wmo_doodads;

  bool draw_doodads_wmo = draw_wmo && draw_wmo_doodads;
  if (draw_doodads_wmo)
  {
    for (auto& wmo : mWMOInstances)
    {
      for (auto& doodad : wmo.second.get_visible_doodads(frustum, culldistance, camera_pos, draw_hidden_models, display))
      {
        _wmo_doodads[doodad->model->filename].push_back(doodad);
      }
    }
  }

  // M2s / models
  if (draw_models || draw_doodads_wmo)
  {
    if (draw_model_animations)
    {
      ModelManager::resetAnim();
    }

    if (need_model_updates)
    {
      update_models_by_filename();
    }

    std::unordered_map<Model*, std::size_t> visible_model_count;

    {
      if (!_m2_program)
      {
        _m2_program.reset(new opengl::program({{ GL_VERTEX_SHADER
          , R"code(
#version 330 core

in vec4 pos;
in vec3 normal;
in vec2 texcoord1;
in vec2 texcoord2;
in mat4 transform;

out vec2 uv1;
out vec2 uv2;
out float camera_dist;
out vec3 norm;

uniform mat4 model_view;
uniform mat4 projection;

uniform int tex_unit_lookup_1;
uniform int tex_unit_lookup_2;

uniform mat4 tex_matrix_1;
uniform mat4 tex_matrix_2;

// code from https://wowdev.wiki/M2/.skin#Environment_mapping
vec2 sphere_map(vec3 vert, vec3 norm)
{
  vec3 normPos = -(normalize(vert));
  vec3 temp = (normPos - (norm * (2.0 * dot(normPos, norm))));
  temp = vec3(temp.x, temp.y, temp.z + 1.0);
 
  return ((normalize(temp).xy * 0.5) + vec2(0.5));
}

vec2 get_texture_uv(int tex_unit_lookup, vec3 vert, vec3 norm)
{
  if(tex_unit_lookup == 0)
  {
    return sphere_map(vert, norm);
  }
  else if(tex_unit_lookup == 1)
  {
    return (transpose(tex_matrix_1) * vec4(texcoord1, 0.0, 1.0)).xy;
  }
  else if(tex_unit_lookup == 2)
  {
    return (transpose(tex_matrix_2) * vec4(texcoord2, 0.0, 1.0)).xy;
  }
  else
  {
    return vec2(0.0);
  }
}

void main()
{
  mat4 camera_mat = model_view * transform;
  vec4 vertex = camera_mat * pos;

  norm = mat3(camera_mat) * normal;

  uv1 = get_texture_uv(tex_unit_lookup_1, vertex.xyz, norm);
  uv2 = get_texture_uv(tex_unit_lookup_2, vertex.xyz, norm);

  camera_dist = -vertex.z;
  gl_Position = projection * vertex;
}
)code"
          }
          ,{ GL_FRAGMENT_SHADER
          , R"code(
#version 330 core

in vec2 uv1;
in vec2 uv2;
in float camera_dist;
in vec3 norm;

out vec4 out_color;

uniform vec4 mesh_color;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform vec4 fog_color;
uniform float fog_start;
uniform float fog_end;
uniform int draw_fog;
uniform int unfogged;
uniform int unlit;

uniform vec3 light_dir;
uniform vec3 diffuse_color;
uniform vec3 ambient_color;

uniform float alpha_test;
uniform int pixel_shader;

vec4 blend_by_alpha (in vec4 source, in vec4 dest)
{
  return source * source.w + dest * (1.0 - source.w);
}

void main()
{
  vec4 color = vec4(0.0);

  if(mesh_color.a < alpha_test)
  {
    discard;
  }

  vec4 texture1 = texture(tex1, uv1);
  vec4 texture2 = texture(tex2, uv2);
  
  // code from Deamon87 and https://wowdev.wiki/M2/Rendering#Pixel_Shaders
  if (pixel_shader == 0) //Combiners_Opaque
  { 
      color.rgb = texture1.rgb * mesh_color.rgb;
      color.a = mesh_color.a;
  } 
  else if (pixel_shader == 1) // Combiners_Decal
  { 
      color.rgb = mix(mesh_color.rgb, texture1.rgb, mesh_color.a);
      color.a = mesh_color.a;
  } 
  else if (pixel_shader == 2) // Combiners_Add
  { 
      color.rgba = texture1.rgba + mesh_color.rgba;
  } 
  else if (pixel_shader == 3) // Combiners_Mod2x
  { 
      color.rgb = texture1.rgb * mesh_color.rgb * vec3(2.0);
      color.a = texture1.a * mesh_color.a * 2.0;
  } 
  else if (pixel_shader == 4) // Combiners_Fade
  { 
      color.rgb = mix(texture1.rgb, mesh_color.rgb, mesh_color.a);
      color.a = mesh_color.a;
  } 
  else if (pixel_shader == 5) // Combiners_Mod
  { 
      color.rgba = texture1.rgba * mesh_color.rgba;
  } 
  else if (pixel_shader == 6) // Combiners_Opaque_Opaque
  { 
      color.rgb = texture1.rgb * texture2.rgb * mesh_color.rgb;
      color.a = mesh_color.a;
  } 
  else if (pixel_shader == 7) // Combiners_Opaque_Add
  { 
      color.rgb = texture2.rgb + texture1.rgb * mesh_color.rgb;
      color.a = mesh_color.a + texture1.a;
  } 
  else if (pixel_shader == 8) // Combiners_Opaque_Mod2x
  { 
      color.rgb = texture1.rgb * mesh_color.rgb * texture2.rgb * vec3(2.0);
      color.a  = texture2.a * mesh_color.a * 2.0;
  } 
  else if (pixel_shader == 9)  // Combiners_Opaque_Mod2xNA
  {
      color.rgb = texture1.rgb * mesh_color.rgb * texture2.rgb * vec3(2.0);
      color.a  = mesh_color.a;
  } 
  else if (pixel_shader == 10) // Combiners_Opaque_AddNA
  { 
      color.rgb = texture2.rgb + texture1.rgb * mesh_color.rgb;
      color.a = mesh_color.a;
  } 
  else if (pixel_shader == 11) // Combiners_Opaque_Mod
  { 
      color.rgb = texture1.rgb * texture2.rgb * mesh_color.rgb;
      color.a = texture2.a * mesh_color.a;
  } 
  else if (pixel_shader == 12) // Combiners_Mod_Opaque
  { 
      color.rgb = texture1.rgb * texture2.rgb * mesh_color.rgb;
      color.a = texture1.a;
  } 
  else if (pixel_shader == 13) // Combiners_Mod_Add
  { 
      color.rgba = texture2.rgba + texture1.rgba * mesh_color.rgba;
  } 
  else if (pixel_shader == 14) // Combiners_Mod_Mod2x
  { 
      color.rgba = texture1.rgba * texture2.rgba * mesh_color.rgba * vec4(2.0);
  } 
  else if (pixel_shader == 15) // Combiners_Mod_Mod2xNA
  { 
      color.rgb = texture1.rgb * texture2.rgb * mesh_color.rgb * vec3(2.0);
      color.a = texture1.a * mesh_color.a;
  } 
  else if (pixel_shader == 16) // Combiners_Mod_AddNA
  { 
      color.rgb = texture2.rgb + texture1.rgb * mesh_color.rgb;
      color.a = texture1.a * mesh_color.a;
  } 
  else if (pixel_shader == 17) // Combiners_Mod_Mod
  { 
      color.rgba = texture1.rgba * texture2.rgba * mesh_color.rgba;
  } 
  else if (pixel_shader == 18) // Combiners_Add_Mod
  { 
      color.rgb = (texture1.rgb + mesh_color.rgb) * texture2.a;
      color.a = (texture1.a + mesh_color.a) * texture2.a;
  } 
  else if (pixel_shader == 19) // Combiners_Mod2x_Mod2x
  {
      color.rgba = texture1.rgba * texture2.rgba * mesh_color.rgba * vec4(4.0);
  }
  else if (pixel_shader == 20)  // Combiners_Opaque_Mod2xNA_Alpha
  {
    color.rgb = (mesh_color.rgb * texture1.rgb) * mix(texture2.rgb * 2.0, vec3(1.0), texture1.a);
    color.a = mesh_color.a;
  }
  else if (pixel_shader == 21)   //Combiners_Opaque_AddAlpha
  {
    color.rgb = (mesh_color.rgb * texture1.rgb) + (texture2.rgb * texture2.a);
    color.a = mesh_color.a;
  }
  else if (pixel_shader == 22)   // Combiners_Opaque_AddAlpha_Alpha
  {
    color.rgb = (mesh_color.rgb * texture1.rgb) + (texture2.rgb * texture2.a * texture1.a);
    color.a = mesh_color.a;
  }

  if(color.a < alpha_test)
  {
    discard;
  }

  if(unlit == 0)
  {
    // diffuse + ambient lighting  
    color.rgb *= vec3(clamp (diffuse_color * max(dot(norm, light_dir), 0.0), 0.0, 1.0)) + ambient_color;
  }  

  if(draw_fog == 1 && unfogged == 0 && camera_dist >= fog_end * fog_start)
  {
    float start = fog_end * fog_start;
    float alpha = (camera_dist - start) / (fog_end - start);
    color = blend_by_alpha (vec4(fog_color.rgb, alpha), color);
  }

  out_color = color;
}

)code"
          }
        }));
      }
      opengl::scoped::use_program m2_shader {*_m2_program.get()};

      m2_shader.uniform("model_view", model_view);
      m2_shader.uniform("projection", projection);
      m2_shader.uniform("tex1", 0);
      m2_shader.uniform("tex2", 1);

      m2_shader.uniform("fog_color", math::vector_4d(skies->color_set[FOG_COLOR], 1));
      // !\ todo use light dbcs values
      m2_shader.uniform("fog_end", fogdistance);
      m2_shader.uniform("fog_start", 0.5f);
      m2_shader.uniform("draw_fog", (int)draw_fog);

      math::vector_3d dd = outdoorLightStats.dayDir;
      math::vector_3d diffuse_color(skies->color_set[LIGHT_GLOBAL_DIFFUSE] * outdoorLightStats.dayIntensity);
      math::vector_3d ambient_color(skies->color_set[LIGHT_GLOBAL_AMBIENT] * outdoorLightStats.ambientIntensity);

      m2_shader.uniform("light_dir", math::vector_3d(-dd.x, -dd.z, dd.y));
      m2_shader.uniform("diffuse_color", diffuse_color);
      m2_shader.uniform("ambient_color", ambient_color);

      if (draw_models)
      {
        for (auto& it : _models_by_filename)
        {
          if (draw_hidden_models || !it.second[0]->model->is_hidden())
          {
            it.second[0]->model->draw(model_view, it.second, m2_shader, frustum, culldistance, camera_pos, false, animtime, false, draw_models_with_box, visible_model_count, display);
          }
        }
      }
      
      if (draw_doodads_wmo)
      {
        for (auto& it : _wmo_doodads)
        {
          it.second[0]->model->draw(model_view, it.second, m2_shader, frustum, culldistance, camera_pos, false, animtime, false, draw_models_with_box, visible_model_count, display);
        }
      }
    } 
    
    if(draw_models_with_box || (draw_hidden_models && !visible_model_count.empty()))
    {
      if (!_m2_box_program)
      {
        _m2_box_program.reset(new opengl::program({ { GL_VERTEX_SHADER
          , R"code(
#version 330 core

in mat4 transform;
in vec4 position;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * model_view * transform * position;
}
)code"
          }
          ,{ GL_FRAGMENT_SHADER
          , R"code(
#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
  out_color = color;
}
)code"
          }
        }));
      }

      opengl::scoped::use_program m2_box_shader{ *_m2_box_program.get() };

      m2_box_shader.uniform ("model_view", model_view);
      m2_box_shader.uniform ("projection", projection);

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);
      gl.lineWidth (1.0f);

      for (auto& it : visible_model_count)
      {

        math::vector_4d color = it.first->is_hidden() 
                              ? math::vector_4d(0.f, 0.f, 1.f, 1.f) 
                              : ( it.first->use_fake_geometry() 
                                ? math::vector_4d(1.f, 0.f, 0.f, 1.f)
                                : math::vector_4d(0.75f, 0.75f, 0.75f, 1.f)
                                )
                              ;

        m2_box_shader.uniform("color", color);
        it.first->draw_box(m2_box_shader, it.second);
      }
    }

    if (IsSelection (eEntry_Model))
    {
      auto model = boost::get<selected_model_type> (*GetCurrentSelection());
      if (model->is_visible(frustum, culldistance, camera_pos, display))
      {
        model->draw_box(model_view, projection, true);
      }
    }
  }
  
  if (!_liquid_render)
  {
    _liquid_render.emplace();
  }

  // todo: find the correct alpha values
  math::vector_4d ocean_color_light(skies->color_set[OCEAN_COLOR_LIGHT], 0.7f);
  math::vector_4d ocean_color_dark (skies->color_set[OCEAN_COLOR_DARK], 0.9f);
  math::vector_4d river_color_light(skies->color_set[RIVER_COLOR_LIGHT], 0.7f);
  math::vector_4d river_color_dark (skies->color_set[RIVER_COLOR_DARK], 0.9f);

  // WMOs / map objects
  if (draw_wmo || mapIndex.hasAGlobalWMO())
  {
    for (std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it)
    {
      bool is_hidden = it->second.wmo->is_hidden();
      if (draw_hidden_models || !is_hidden)
      {
        if (!_wmo_program)
        {
          _wmo_program.reset(new opengl::program 
            { 
              { GL_VERTEX_SHADER
              , R"code(
#version 330 core

in vec4 position;
in vec3 normal;
in vec3 color;
in vec2 texcoord;

out vec2 f_texcoord;


uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 transform;

void main()
{
  gl_Position = projection * model_view * transform * position;

  f_texcoord = texcoord;
}
)code"
              }
            , { GL_FRAGMENT_SHADER
              , R"code(
#version 330 core

uniform sampler2D tex1;

in vec2 f_texcoord;

out vec4 out_color;

void main()
{
  out_color = texture2D(tex1, f_texcoord);
}
)code"
                   }
            });
      }

      opengl::scoped::use_program wmo_program {*_wmo_program.get()};

      wmo_program.uniform("model_view", model_view);
      wmo_program.uniform("projection", projection);
      wmo_program.uniform("tex1", 0);

      it->second.draw( wmo_program
                     , model_view
                     , projection
                     , frustum
                     , culldistance
                     , camera_pos
                     , is_hidden
                     , draw_wmo_doodads
                     , draw_fog
                     , ocean_color_light
                     , ocean_color_dark
                     , river_color_light
                     , river_color_dark
                     , _liquid_render.get()
                     , mCurrentSelection
                     , animtime
                     , skies->hasSkies()
                     , display
                     );
      }
    }
  }

  outdoorLights(true);
  setupFog (draw_fog);

  gl.color4f(1, 1, 1, 1);
  gl.enable(GL_BLEND);

  if (draw_water)
  {
    // draw the water on both sides
    opengl::scoped::bool_setter<GL_CULL_FACE, FALSE> const cull;

    opengl::scoped::use_program water_shader{ _liquid_render->shader_program() };

    water_shader.uniform ("model_view", model_view);
    water_shader.uniform ("projection", projection);

    water_shader.uniform ("use_transform", 0);

    water_shader.uniform ("ocean_color_light", ocean_color_light);
    water_shader.uniform ("ocean_color_dark",  ocean_color_dark);
    water_shader.uniform ("river_color_light", river_color_light);
    water_shader.uniform ("river_color_dark",  river_color_dark);

    
    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawWater ( frustum
                      , culldistance
                      , camera_pos
                      , _liquid_render.get()
                      , water_shader
                      , animtime
                      , water_layer
                      , display
                      );
    }
  }

  // draw last because of the transparency
  if (draw_mfbo)
  {
    // don't write on the depth buffer
    gl.depthMask(GL_FALSE);

    if (!_mfbo_program)
    {
      _mfbo_program.reset(new opengl::program({{ GL_VERTEX_SHADER
        , R"code(
#version 330 core

attribute vec4 position;

uniform mat4 model_view_projection;

void main()
{
  gl_Position = model_view_projection * position;
}
)code"
        }
        ,{ GL_FRAGMENT_SHADER
        , R"code(
#version 330 core

uniform vec4 color;

out vec4 out_color;

void main()
{
  out_color = color;
}
)code"
        }
                          }));
    }
    opengl::scoped::use_program mfbo_shader {*_mfbo_program.get()};

    mfbo_shader.uniform("model_view_projection", model_view * projection);

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawMFBO(mfbo_shader);
    }

    gl.depthMask(GL_TRUE);
  }
}

selection_result World::intersect ( math::matrix_4x4 const& model_view
                                  , math::ray const& ray
                                  , bool pOnlyMap
                                  , bool do_objects
                                  , bool draw_terrain
                                  , bool draw_wmo
                                  , bool draw_models
                                  , bool draw_hidden_models
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
        if (draw_hidden_models || ! model_instance.second.model->is_hidden())
        {
          model_instance.second.intersect (model_view, ray, &results, animtime);
        }
      }
    }

    if (draw_wmo)
    {
      for (auto&& wmo_instance : mWMOInstances)
      {
        if (draw_hidden_models || ! wmo_instance.second.wmo->is_hidden())
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

  update_models_by_filename();
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

bool World::GetVertex(float x, float z, math::vector_3d *V) const
{
  tile_index tile({x, 0, z});

  if (!mapIndex.tileLoaded(tile))
  {
    return false;
  }

  MapTile* adt = mapIndex.getTile(tile);

  return adt->finishedLoading() && adt->GetVertex(x, z, V);
}

template<typename Fun>
  bool World::for_all_chunks_in_range (math::vector_3d const& pos, float radius, Fun&& fun)
{
  bool changed (false);

  for (MapTile* tile : mapIndex.tiles_in_range (pos, radius))
  {
    if (!tile->finishedLoading())
    {
      continue;
    }

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

bool World::replaceTexture(math::vector_3d const& pos, float radius, scoped_blp_texture_reference old_texture, scoped_blp_texture_reference new_texture)
{
  return for_all_chunks_in_range
    ( pos, radius
      , [&](MapChunk* chunk)
      {
        return chunk->replaceTexture(pos, radius, old_texture, new_texture);
      }
    );
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
  if (tile && tile->finishedLoading())
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
    if (tile && tile->finishedLoading())
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

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        AsyncLoader::instance().ensure_loaded(mTile);

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
  throw std::runtime_error("minimap saving not implemented");
}

void World::deleteModelInstance(int pUniqueID)
{
  std::map<int, ModelInstance>::iterator it = mModelInstances.find(pUniqueID);
  if (it == mModelInstances.end()) return;

  updateTilesModel(&it->second, model_update::remove);
  mModelInstances.erase(it);
  ResetSelection();
}

void World::deleteWMOInstance(int pUniqueID)
{
  std::map<int, WMOInstance>::iterator it = mWMOInstances.find(pUniqueID);
  if (it == mWMOInstances.end()) return;

  updateTilesWMO(&it->second, model_update::remove);
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
        && lhs->second.wmo->filename == rhs->second.wmo->filename
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
        && lhs->second.model->filename == rhs->second.model->filename
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

  update_models_by_filename();

  Log << "Deleted " << wmos_to_remove.size() << " duplicate WMOs" << std::endl;
  Log << "Deleted " << models_to_remove.size() << " duplicate models" << std::endl;
}

void World::warning_if_uid_in_use(uint32_t uid)
{
  if(mModelInstances.find(uid) != mModelInstances.end() || mWMOInstances.find(uid) != mWMOInstances.end())
  {
    QMessageBox::critical( nullptr
                         , "UID ALREADY IN USE"
                         , "Please enable 'Always check for max UID', mysql uid store or synchronize your "
                           "uid.ini file if you're sharing the map between several mappers.\n"
                           "Use 'Editor > Force uid check on next opening' to fix the issue."
                         );
  }
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

  if (_settings->value("model/random_rotation", false).toBool())
  {
    float min = paste_params->minRotation;
    float max = paste_params->maxRotation;
    newModelis.dir.y += misc::randfloat(min, max);
  }

  if (_settings->value ("model/random_tilt", false).toBool ())
  {
    float min = paste_params->minTilt;
    float max = paste_params->maxTilt;
    newModelis.dir.x += misc::randfloat(min, max);
    newModelis.dir.z += misc::randfloat(min, max);
  }

  if (_settings->value ("model/random_size", false).toBool ())
  {
    float min = paste_params->minScale;
    float max = paste_params->maxScale;
    newModelis.scale = misc::randfloat(min, max);
  }

  // to ensure the tiles are updated correctly
  AsyncLoader::instance().ensure_loaded(newModelis.model.get());
  newModelis.recalcExtents();
  updateTilesModel(&newModelis, model_update::add);

  warning_if_uid_in_use(newModelis.uid);
  
  _models_by_filename[filename].push_back(&(mModelInstances.emplace(newModelis.uid, newModelis).first->second));
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

  // to ensure the tiles are updated correctly
  AsyncLoader::instance().ensure_loaded(newWMOis.wmo.get());

  // recalc the extends
  newWMOis.recalcExtents();
  updateTilesWMO(&newWMOis, model_update::add);

  warning_if_uid_in_use(newWMOis.mUniqueID);

  mWMOInstances.emplace(newWMOis.mUniqueID, newWMOis);
}

void World::remove_models_if_needed(std::vector<uint32_t> const& uids, tile_index const& tile_unloading)
{
  for (uint32_t uid : uids)
  {
    bool remove = true;

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      if (tile->has_model(uid))
      {
        remove = false;
        break;
      }
    }

    if (remove)
    {
      if (mModelInstances.find(uid) != mModelInstances.end())
      {
        mModelInstances.erase(uid);
      }
      else
      {
        mWMOInstances.erase(uid);
      }
    }
  }

  // todo: only reset if the selected model has been unloaded
  ResetSelection();
  update_models_by_filename();
}

void World::reload_tile(tile_index const& tile)
{
  ResetSelection();
  // to remove the new models and reload the old ones
  clearAllModelsOnADT(tile);
  mapIndex.reloadTile(tile);
}

void World::updateTilesEntry(selection_type const& entry, model_update type)
{
  if (entry.which() == eEntry_WMO)
  {
    updateTilesWMO (boost::get<selected_wmo_type> (entry), type);
  }
  else if (entry.which() == eEntry_Model)
  {
    updateTilesModel (boost::get<selected_model_type> (entry), type);
  }
}

void World::updateTilesWMO(WMOInstance* wmo, model_update type)
{
  tile_index start(wmo->extents[0]), end(wmo->extents[1]);
  for (int z = start.z; z <= end.z; ++z)
  {
    for (int x = start.x; x <= end.x; ++x)
    {
      mapIndex.update_model_tile(tile_index(x, z), type, wmo->mUniqueID);     
    }
  }
}

void World::updateTilesModel(ModelInstance* m2, model_update type)
{
  auto const& extents(m2->extents());
  tile_index start(extents[0]), end(extents[1]);

  for (int z = start.z; z <= end.z; ++z)
  {
    for (int x = start.x; x <= end.x; ++x)
    {
      mapIndex.update_model_tile(tile_index(x, z), type, m2->uid);
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
  for_all_chunks_on_tile(pos, [](MapChunk* chunk) { chunk->texture_set->removeDuplicate(); } );
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

void World::update_models_by_filename()
{
  _models_by_filename.clear();
  
  for (auto& it : mModelInstances)
  {
    _models_by_filename[it.second.model->filename].push_back(&it.second);
    // to make sure the transform matrix are up to date
    it.second.recalcExtents();
  }

  need_model_updates = false;
}
