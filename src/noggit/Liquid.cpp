// Liquid.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/Liquid.h>

#include <algorithm>
#include <string>

#include <noggit/blp_texture.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Shaders.h>
#include <noggit/World.h>
#include <noggit/mpq/file.h>

#include <opengl/call_list.h>
#include <opengl/texture.h>

#include <boost/filesystem/path.hpp>

namespace
{
#ifdef USEBLSFILES
  BLSShader* mWaterShader;
  BLSShader* mMagmaShader;
#else
  opengl::shader waterShader;
  opengl::shader waterFogShader;
#endif
}

void loadWaterShader()
{
#ifdef USEBLSFILES
  mWaterShader = new BLSShader( "shaders\\pixel\\arbfp1\\psLiquidWater.bls" );
  mMagmaShader = new BLSShader( "shaders\\pixel\\arbfp1\\psLiquidMagma.bls" );
#else
  boost::filesystem::path waterPath ("shaders/water.ps");
  boost::filesystem::path fogPath ("shaders/waterfog.ps");

  waterPath.make_preferred();
  fogPath.make_preferred();

  FILE *shader = fopen(waterPath.string().c_str(), "r");

  if( !shader )
  {
    LogError << "Unable to open water shader " << waterPath.string() << std::endl;
  }
  else
  {
    char buffer[8192];
    int length=fread(buffer, 1, 8192, shader);
    fclose(shader);
    glGenProgramsARB(1, &waterShader);
    if(waterShader==0)
      LogError << "Failed to get program ID for water shader " << waterPath.string() << "." << std::endl;
    else
    {
      GLint errorPos, isNative;

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
      glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

      glGetProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
      if( !(errorPos==-1)&&(isNative==1) )
      {
        int i, j;
        char localbuffer[256];
        LogError << "Water Shader " << waterPath.string() << " Fragment program failed to load \nReason:\n";
        LogError << reinterpret_cast<const char*>(glGetString(GL_PROGRAM_ERROR_STRING_ARB)) << std::endl;
        for(i=errorPos, j=0; (i<length)&&(j<128); ++i, j++)
        {
          localbuffer[j]=buffer[i];
        }
        localbuffer[j]=0;
        LogError << "START DUMP :" << std::endl << localbuffer << "END DUMP" << std::endl;
        if(isNative==0)
          LogError << "This fragment program exceeded the limit." << std::endl;
      }
    }
  }

  shader = fopen(fogPath.string().c_str(), "r");
  if(shader==0)
    LogError << "Unable to open water shader " << fogPath.string() << "." << std::endl;
  else
  {
    char buffer[8192];
    int length=fread(buffer, 1, 8192, shader);
    fclose(shader);
    glGenProgramsARB(1, &waterFogShader);
    if(waterFogShader==0)
      LogError << "Failed to get program ID for water shader " << fogPath.string() << "." << std::endl;
    else
    {
      GLint errorPos, isNative;

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
      glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

      glGetProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
      if( !(errorPos==-1)&&(isNative==1) )
      {
        int i, j;
        //const GLubyte *stringy;
        char localbuffer[256];
        LogError << "Water Shader " << fogPath.string() << " Fragment program failed to load \nReason:\n";
        LogError << reinterpret_cast<const char*>(glGetString(GL_PROGRAM_ERROR_STRING_ARB)) << std::endl;
        for(i=errorPos, j=0; (i<length)&&(j<128); ++i, j++)
        {
          localbuffer[j]=buffer[i];
        }
        localbuffer[j]=0;
        LogError << "START DUMP :" << std::endl << localbuffer << "END DUMP" << std::endl;
        if(isNative==0)
          LogError << "This fragment program exceeded the limit." << std::endl;
      }
    }
  }
#endif
}

Liquid::Liquid(int x, int y, ::math::vector_3d base, float ptilesize)
  : xtiles(x)
  , ytiles(y)
  , pos(base)
  , tilesize(ptilesize)
  , ydir(1.0f)
{}

Liquid::~Liquid() = default;

void Liquid::initFromWMO(noggit::mpq::file* f, const WMOMaterial &mat, bool indoor)
{
  texRepeats = 4.0f;
  ydir = -1.0f;

  initGeometry(f);

  trans = false;

  // tmpflag is the flags value for the last drawn tile
  if (tmpflag & 1) {
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    type = 0;
    texRepeats = 2.0f;
    mTransparency = false;
  }
  else if (tmpflag & 2) {
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    type = 0;
    mTransparency = false;
  }
  else {
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    if (indoor) {
      trans = true;
      type = 1;
      col = ::math::vector_3d( ( ( mat.col2 & 0xFF0000 ) >> 16 ) / 255.0f, ( ( mat.col2 & 0xFF00 ) >> 8 ) / 255.0f, ( mat.col2 & 0xFF ) / 255.0f);
    } else {
      trans = true;
      type = 2; // outdoor water (...?)
    }
    mTransparency = true;
  }

}


void Liquid::initGeometry(noggit::mpq::file* f)
{
  LiquidVertex *map = reinterpret_cast<LiquidVertex*>(f->getPointer());
  unsigned char *flags = reinterpret_cast<unsigned char*>(f->getPointer() + (xtiles+1)*(ytiles+1)*sizeof(LiquidVertex));

  // generate vertices
  ::math::vector_3d * lVertices = new ::math::vector_3d[(xtiles+1)*(ytiles+1)];
  for (int j=0; j<ytiles+1; j++) {
    for (int i=0; i<xtiles+1; ++i) {
      size_t p = j*(xtiles+1)+i;
      float h = map[p].h;
      if (h > 100000) h = pos.y();
            lVertices[p] = ::math::vector_3d(pos.x() + tilesize * i, h, pos.z() + ydir * tilesize * j);
    }
  }

  mDrawList.reset (new opengl::call_list);
  mDrawList->start_recording();

  //! \todo  handle light/dark liquid colors
  glNormal3f(0, 1, 0);
  glBegin(GL_QUADS);
  // draw tiles
  for (int j=0; j<ytiles; j++) {
    for (int i=0; i<xtiles; ++i) {
      unsigned char flag = flags[j*xtiles+i];
      if ( !( flag & 8 ) )
      {
        tmpflag = flag;
        // 15 seems to be "don't draw"
        size_t p = j*(xtiles+1)+i;

        float c;

        c=static_cast<float>(map[p].c[0])/255.0f;
        glMultiTexCoord2f(GL_TEXTURE1,c,c);
        glTexCoord2f(i / texRepeats, j / texRepeats);
        glVertex3fv(lVertices[p]);

        c=static_cast<float>(map[p+1].c[0])/255.0f;
        glMultiTexCoord2f(GL_TEXTURE1,c,c);
        glTexCoord2f((i+1) / texRepeats, j / texRepeats);
        glVertex3fv(lVertices[p+1]);

        c=static_cast<float>(map[p+xtiles+1+1].c[0])/255.0f;
        glMultiTexCoord2f(GL_TEXTURE1,c,c);
        glTexCoord2f((i+1) / texRepeats, (j+1) / texRepeats);
        glVertex3fv(lVertices[p+xtiles+1+1]);

        c=static_cast<float>(map[p+xtiles+1].c[0])/255.0f;
        glMultiTexCoord2f(GL_TEXTURE1,c,c);
        glTexCoord2f(i / texRepeats, (j+1) / texRepeats);
        glVertex3fv(lVertices[p+xtiles+1]);
      }
    }
  }
  glEnd();

  mDrawList->end_recording();

  delete[] lVertices;
  lVertices = nullptr;
}

void Liquid::initFromMH2O (MH2O_Tile const& tile_info)
{
  texRepeats = 4.0f;
  ydir = 1.0f;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(tile_info.mLiquidType);
    initTextures<1, 30>(lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames - 1));

    mLiquidType = lLiquidTypeRow.getInt(LiquidTypeDB::Type);
    mShaderType = lLiquidTypeRow.getInt(LiquidTypeDB::ShaderType);
    //mLiquidType = 0;
    //mShaderType = 1;
    //! \todo  Get texRepeats too.
  }
  catch (...)
  {
    // Fallback, when there is no information.
    initTextures<1, 30>("XTextures\\river\\lake_a.%d.blp");
    mLiquidType = 0;
    mShaderType = 1;
  }

  mTransparency = mShaderType & 1;

  // generate vertices
  ::math::vector_3d lVertices[9][9];
  for (int j = 0; j < 9; ++j)
  {
    for (int i = 0; i < 9; ++i)
    {
      lVertices[j][i] = ::math::vector_3d(pos.x() + tilesize * i, tile_info.mHeightmap[j][i], pos.z() + ydir * tilesize * j);
    }
  }

  mDrawList.reset (new opengl::call_list);
  mDrawList->start_recording();

  glBegin(GL_QUADS);

  glNormal3f(0.0f, 1.0f, 0.0f);

  // draw tiles
  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < 8; ++i)
    {
      if (!tile_info.mRender[j][i]) continue;

      float c;
      c = tile_info.mDepth[j][i];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      glTexCoord2f(i / texRepeats, j / texRepeats);
      glVertex3fv(lVertices[j][i]);

      c = tile_info.mDepth[j][i + 1];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      glTexCoord2f((i + 1) / texRepeats, j / texRepeats);
      glVertex3fv(lVertices[j][i + 1]);

      c = tile_info.mDepth[j + 1][i + 1];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      glTexCoord2f((i + 1) / texRepeats, (j + 1) / texRepeats);
      glVertex3fv(lVertices[j + 1][i + 1]);

      c = tile_info.mDepth[j + 1][i];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      glTexCoord2f(i / texRepeats, (j + 1) / texRepeats);
      glVertex3fv(lVertices[j + 1][i]);
    }
  }


  glEnd();

  mDrawList->end_recording();
}

namespace
{
  //! \todo move to opengl::, wrap everything
  void CheckForGLError (std::string const& pLocation)
  {
    while (int ErrorNum = glGetError())
    {
      switch (ErrorNum)
      {
        case GL_INVALID_ENUM:
           LogError << "OpenGL: (at " << pLocation << "): GL_INVALID_ENUM" << std::endl;
           break;
        case GL_INVALID_VALUE:
           LogError << "OpenGL: (at " << pLocation << "): GL_INVALID_VALUE" << std::endl;
           break;
         case GL_INVALID_OPERATION:
           LogError << "OpenGL: (at " << pLocation << "): GL_INVALID_OPERATION" << std::endl;
           break;
         case GL_STACK_OVERFLOW:
           LogError << "OpenGL: (at " << pLocation << "): GL_STACK_OVERFLOW" << std::endl;
           break;
         case GL_STACK_UNDERFLOW:
           LogError << "OpenGL: (at " << pLocation << "): GL_STACK_UNDERFLOW" << std::endl;
           break;
         case GL_OUT_OF_MEMORY:
           LogError << "OpenGL: (at " << pLocation << "): GL_OUT_OF_MEMORY" << std::endl;
           break;
         case GL_TABLE_TOO_LARGE:
           LogError << "OpenGL: (at " << pLocation << "): GL_TABLE_TOO_LARGE" << std::endl;
           break;
         case GL_NO_ERROR:
        //! \todo  Add the missing ones.
         default:
           LogError << "OpenGL: (at " << pLocation << "): GL_NO_ERROR (wat?)" << std::endl;
       }

      ErrorNum = glGetError();
    }
  }
}

void Liquid::draw (Skies const* skies) const
{
  glEnable(GL_FRAGMENT_PROGRAM_ARB);

#ifdef USEBLSFILES
  if( type == 2 && mWaterShader->IsOkay() )
    mWaterShader->EnableShader();
  if( type == 0 && mMagmaShader->IsOkay() )
    mMagmaShader->EnableShader();
#else
  if(glIsEnabled(GL_FOG)==GL_TRUE)
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
  else
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
#endif

  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
  size_t texidx = (size_t)(clock() / CLOCKS_PER_SEC / 60.0f) % _textures.size();

  if( mTransparency )
  {
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
  }

  const float tcol = mTransparency ? 0.85f : 1.0f;
  ::math::vector_3d col = skies->colorSet[WATER_COLOR_LIGHT] * 0.7f; //! \todo  add variable water color
  ::math::vector_3d col2 = skies->colorSet[WATER_COLOR_DARK] * 0.2f;

  glColor4f (col.x(), col.y(), col.z(), tcol);
  glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, col2.x(), col2.y(), col2.z(), tcol);
  opengl::texture::enable_texture (0);

  _textures[texidx]->bind();

  opengl::texture::enable_texture (1);

  if (mDrawList)
  {
    //! \todo THIS LINE THROWS GL_INVALID_OPERATION! Steff. It donwt do in anymore now. Perhaps because water rendering was called double in maptile::draw()
    CheckForGLError( "Liquid::draw:: before the draw list" );
    mDrawList->render();
    CheckForGLError("Liquid::draw:: after the draw list");
  }

  opengl::texture::disable_texture (1);
  opengl::texture::set_active_texture (0);

  glColor4f(1, 1, 1, 0.4f);
  if (mTransparency)
  {
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
  glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

template<int pFirst, int pLast>
void Liquid::initTextures( const std::string& pFilename )
{
  for( int i = pFirst; i <= pLast; ++i )
  {
    QString tmp (QString::fromStdString(pFilename));
    tmp = tmp.replace("%d", "%1");
    tmp = tmp.arg (i);
    _textures.emplace_back (tmp.toStdString());
  }
}
