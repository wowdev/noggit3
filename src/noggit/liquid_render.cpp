// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <noggit/Shaders.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>

#ifdef USEBLSFILES
BLSShader * mWaterShader;
BLSShader * mMagmaShader;
#else
OpenGL::Shader  waterShader;
OpenGL::Shader  waterFogShader;
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
  glEnable(GL_FRAGMENT_PROGRAM_ARB);
#ifdef USEBLSFILES
  if (type == 2 && mWaterShader->IsOkay())
    mWaterShader->EnableShader();
  if (type == 0 && mMagmaShader->IsOkay())
    mMagmaShader->EnableShader();
#else
  enableWaterShader();
#endif

  math::vector_3d col2;
  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
  size_t texidx = (size_t)(gWorld->animtime / 60.0f) % _textures.size();

  const float tcol = _transparency ? 0.85f : 1.0f;

  if (_transparency)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
  }

  _col = gWorld->skies->colorSet[WATER_COLOR_LIGHT] * 0.7f; //! \todo  add variable water color
  col2 = gWorld->skies->colorSet[WATER_COLOR_DARK] * 0.2f;

  glColor4f(_col.x, _col.y, _col.z, tcol);
  glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, col2.x, col2.y, col2.z, tcol);

  opengl::texture::set_active_texture(0);
  opengl::texture::enable_texture();

  _textures[texidx]->bind();

  opengl::texture::set_active_texture(1);
  opengl::texture::enable_texture();

  if (_draw_list)
  {
    //! \todo THIS LINE THROWS GL_INVALID_OPERATION! Steff. It donwt do in anymore now. Perhaps because water rendering was called double in maptile::draw()
    _draw_list->render();
  }

  opengl::texture::set_active_texture(1);
  opengl::texture::disable_texture();
  opengl::texture::set_active_texture(0);

  glColor4f(1, 1, 1, 0.4f);
  if (_transparency)
  {
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
  glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

liquid_render::liquid_render(math::vector_3d const& col, bool transparency, opengl::call_list* draw_list, std::string const& filename)
  : _transparency(transparency)
  , _col(col)
  , _draw_list(draw_list)
{
  setTextures(filename);
}

void liquid_render::setTextures(std::string const& filename)
{
  _textures.clear();

  for (int i = 1; i <= 30; ++i)
  {
    _textures.emplace_back(boost::str(boost::format(filename) % i));
  }
}

void liquid_render::changeDrawList(opengl::call_list* draw_list)
{
  _draw_list.reset(draw_list);
}