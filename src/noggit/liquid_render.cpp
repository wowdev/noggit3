// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <opengl/context.hpp>
#include <opengl/matrix.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>

#ifdef USEBLSFILES
BLSShader * mWaterShader;
BLSShader * mMagmaShader;
#else
GLuint waterShader;
GLuint waterFogShader;
#endif

void loadWaterShader()
{
#ifdef USEBLSFILES
  mWaterShader = new BLSShader("shaders\\pixel\\arbfp1\\psLiquidWater.bls");
  mMagmaShader = new BLSShader("shaders\\pixel\\arbfp1\\psLiquidMagma.bls");
#else
  boost::filesystem::path waterPath("shaders/water.ps");
  boost::filesystem::path fogPath("shaders/waterfog.ps");

  waterPath.make_preferred();
  fogPath.make_preferred();

  FILE *shader = fopen(waterPath.string().c_str(), "r");
  if (!shader)
  {
    LogError << "Unable to open water shader " << waterPath.string() << std::endl;
  }
  else
  {
    char buffer[8192];
    int length = fread(buffer, 1, 8192, shader);
    fclose(shader);
    glGenProgramsARB(1, &waterShader);
    if (!waterShader)
    {
      LogError << "Failed to get program ID for water shader " << waterPath.string() << std::endl;
    }
    else
    {
      GLint errorPos, isNative;

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
      glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

      glGetProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
      if (!(errorPos == -1) && (isNative == 1))
      {
        int i, j;
        char localbuffer[256];
        LogError << "Water Shader \"shaders\\water.ps\" Fragment program failed to load \nReason:\n";
        LogError << reinterpret_cast<const char*>(glGetString(GL_PROGRAM_ERROR_STRING_ARB)) << std::endl;
        for (i = errorPos, j = 0; (i<length) && (j<128); ++i, j++)
        {
          localbuffer[j] = buffer[i];
        }
        localbuffer[j] = 0;
        LogError << "START DUMP :" << std::endl << localbuffer << "END DUMP" << std::endl;
        if (isNative == 0)
          LogError << "This fragment program exceeded the limit." << std::endl;
      }
    }
  }

  shader = fopen(fogPath.string().c_str(), "r");
  if (!shader)
  {
    LogError << "Unable to open water shader " << fogPath.string() << std::endl;
  }
  else
  {
    char buffer[8192];
    int length = fread(buffer, 1, 8192, shader);
    fclose(shader);
    glGenProgramsARB(1, &waterFogShader);
    if (!waterFogShader)
    {
      LogError << "Failed to get program ID for water shader " << fogPath.string() << std::endl;
    }
    else
    {
      GLint errorPos, isNative;

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
      glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

      glGetProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
      if (!(errorPos == -1) && (isNative == 1))
      {
        int i, j;
        //const GLubyte *stringy;
        char localbuffer[256];
        LogError << "Water Shader \"shaders/waterfog.ps\" Fragment program failed to load \nReason:\n";
        LogError << reinterpret_cast<const char*>(glGetString(GL_PROGRAM_ERROR_STRING_ARB)) << std::endl;
        for (i = errorPos, j = 0; (i<length) && (j<128); ++i, j++)
        {
          localbuffer[j] = buffer[i];
        }
        localbuffer[j] = 0;
        LogError << "START DUMP :" << std::endl << localbuffer << "END DUMP" << std::endl;
        if (isNative == 0)
          LogError << "This fragment program exceeded the limit." << std::endl;
      }
    }
  }
#endif
}

#ifndef USEBLSFILES
void enableWaterShader()
{
  if (glIsEnabled(GL_FOG) == GL_TRUE)
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
  else
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
}
#endif


void liquid_render::draw()
{
  if (!_ready)
  {
    return;
  }

  gl.enable(GL_FRAGMENT_PROGRAM_ARB);
#ifdef USEBLSFILES
  if (type == 2 && mWaterShader->IsOkay())
    mWaterShader->EnableShader();
  if (type == 0 && mMagmaShader->IsOkay())
    mMagmaShader->EnableShader();
#else
  enableWaterShader();
#endif

  
  gl.disable(GL_CULL_FACE);
  gl.depthFunc(GL_LESS);
  size_t texidx = (size_t)(gWorld->animtime / 60.0f) % _textures.size();

  const float tcol = _transparency ? 0.85f : 1.0f;

  if (_transparency)
  {
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.depthMask(GL_FALSE);
  }

  math::vector_3d color  = gWorld->skies->colorSet[WATER_COLOR_LIGHT] * 0.7f; //! \todo  add variable water color
  math::vector_3d color2 = gWorld->skies->colorSet[WATER_COLOR_DARK] * 0.2f;

  gl.color4f(color.x, color.y, color.z, tcol);
  glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, color2.x, color2.y, color2.z, tcol);

  opengl::texture::set_active_texture(0);
  opengl::texture::enable_texture();

  _textures[texidx]->bind();

  opengl::texture::set_active_texture(1);
  opengl::texture::enable_texture();

  if(draw_list)
  {
    draw_list->render();
  }

  opengl::texture::set_active_texture(1);
  opengl::texture::disable_texture();
  opengl::texture::set_active_texture(0);

  gl.color4f(1, 1, 1, 0.4f);
  if (_transparency)
  {
    gl.depthMask(GL_TRUE);
    gl.disable(GL_BLEND);
  }
  gl.disable(GL_FRAGMENT_PROGRAM_ARB);
}

void liquid_render::draw (std::function<void (opengl::scoped::use_program&)> actual)
{
  opengl::program const program
    { { GL_VERTEX_SHADER
      , R"code(
#version 110

attribute vec4 position;
attribute vec2 tex_coord;
attribute float depth;

uniform mat4 model_view;
uniform mat4 projection;

varying float depth_;
varying vec2 tex_coord_;

void main()
{
  depth_ = depth;
  tex_coord_ = tex_coord;

  gl_Position = projection * model_view * position;
}
)code"
      }
    , { GL_FRAGMENT_SHADER
      , R"code(
#version 110

uniform sampler2D texture;
uniform vec4 color_light;
uniform vec4 color_dark;
uniform float tex_repeat;

varying float depth_;
varying vec2 tex_coord_;

void main()
{
  vec4 texel = texture2D (texture, tex_coord_ / tex_repeat);
  vec4 lerp = mix (color_dark, color_light, depth_);
  vec4 tResult = clamp (texel + lerp, 0.0, 1.0); //clamp shouldn't be needed
  vec4 oColor = clamp (texel + tResult, 0.0, 1.0);
  gl_FragColor = vec4 (oColor.rgb, lerp.a);
}
)code"
      }
    };

  opengl::scoped::use_program water_shader {program};

  water_shader.uniform ("model_view", opengl::matrix::model_view());
  water_shader.uniform ("projection", opengl::matrix::projection());

  water_shader.uniform ("color_light", {gWorld->skies->colorSet[WATER_COLOR_LIGHT], 1.f});
  water_shader.uniform ("color_dark", {gWorld->skies->colorSet[WATER_COLOR_DARK], 1.f});

  water_shader.sampler
    ( "texture"
    , GL_TEXTURE0
    , _textures[static_cast<std::size_t> (gWorld->animtime / 60.0f) % _textures.size()].get()
    );

  actual (water_shader);
}

liquid_render::liquid_render(bool transparency, std::string const& filename)
  : _transparency(transparency)
  , draw_list(nullptr)
{
  if (filename != "")
  {
    setTextures(filename);
  }
  
  _ready = !_textures.empty();
}

void liquid_render::setTextures(std::string const& filename)
{
  _textures.clear();

  for (int i = 1; i <= 30; ++i)
  {
    _textures.emplace_back(boost::str(boost::format(filename) % i));
  }

  _ready = true;
}