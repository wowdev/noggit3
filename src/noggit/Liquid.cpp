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
#include <opengl/context.hpp>
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
    gl.genPrograms(1, &waterShader);
    if(waterShader==0)
      LogError << "Failed to get program ID for water shader " << waterPath.string() << "." << std::endl;
    else
    {
      GLint errorPos, isNative;

      gl.bindProgram(GL_FRAGMENT_PROGRAM_ARB, waterShader);
      gl.programString(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
      gl.getIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

      gl.getProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
      if( !(errorPos==-1)&&(isNative==1) )
      {
        int i, j;
        char localbuffer[256];
        LogError << "Water Shader " << waterPath.string() << " Fragment program failed to load \nReason:\n";
        LogError << reinterpret_cast<const char*>(gl.getString(GL_PROGRAM_ERROR_STRING_ARB)) << std::endl;
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
    gl.genPrograms(1, &waterFogShader);
    if(waterFogShader==0)
      LogError << "Failed to get program ID for water shader " << fogPath.string() << "." << std::endl;
    else
    {
      GLint errorPos, isNative;

      gl.bindProgram(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
      gl.programString(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
      gl.getIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

      gl.getProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
      if( !(errorPos==-1)&&(isNative==1) )
      {
        int i, j;
        //const GLubyte *stringy;
        char localbuffer[256];
        LogError << "Water Shader " << fogPath.string() << " Fragment program failed to load \nReason:\n";
        LogError << reinterpret_cast<const char*>(gl.getString(GL_PROGRAM_ERROR_STRING_ARB)) << std::endl;
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
  gl.normal3f(0, 1, 0);
  gl.begin(GL_QUADS);
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
        gl.multiTexCoord2f(GL_TEXTURE1,c,c);
        gl.texCoord2f(i / texRepeats, j / texRepeats);
        gl.vertex3fv(lVertices[p]);

        c=static_cast<float>(map[p+1].c[0])/255.0f;
        gl.multiTexCoord2f(GL_TEXTURE1,c,c);
        gl.texCoord2f((i+1) / texRepeats, j / texRepeats);
        gl.vertex3fv(lVertices[p+1]);

        c=static_cast<float>(map[p+xtiles+1+1].c[0])/255.0f;
        gl.multiTexCoord2f(GL_TEXTURE1,c,c);
        gl.texCoord2f((i+1) / texRepeats, (j+1) / texRepeats);
        gl.vertex3fv(lVertices[p+xtiles+1+1]);

        c=static_cast<float>(map[p+xtiles+1].c[0])/255.0f;
        gl.multiTexCoord2f(GL_TEXTURE1,c,c);
        gl.texCoord2f(i / texRepeats, (j+1) / texRepeats);
        gl.vertex3fv(lVertices[p+xtiles+1]);
      }
    }
  }
  gl.end();

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

  gl.begin(GL_QUADS);

  gl.normal3f(0.0f, 1.0f, 0.0f);

  // draw tiles
  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < 8; ++i)
    {
      if (!tile_info.mRender[j][i]) continue;

      float c;
      c = tile_info.mDepth[j][i];
      gl.multiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, j / texRepeats);
      gl.vertex3fv(lVertices[j][i]);

      c = tile_info.mDepth[j][i + 1];
      gl.multiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, j / texRepeats);
      gl.vertex3fv(lVertices[j][i + 1]);

      c = tile_info.mDepth[j + 1][i + 1];
      gl.multiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(lVertices[j + 1][i + 1]);

      c = tile_info.mDepth[j + 1][i];
      gl.multiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(lVertices[j + 1][i]);
    }
  }


  gl.end();

  mDrawList->end_recording();
}

void Liquid::draw (Skies const* skies) const
{
  gl.enable(GL_FRAGMENT_PROGRAM_ARB);

#ifdef USEBLSFILES
  if( type == 2 && mWaterShader->IsOkay() )
    mWaterShader->EnableShader();
  if( type == 0 && mMagmaShader->IsOkay() )
    mMagmaShader->EnableShader();
#else
  if(gl.isEnabled(GL_FOG)==GL_TRUE)
    gl.bindProgram(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
  else
    gl.bindProgram(GL_FRAGMENT_PROGRAM_ARB, waterShader);
#endif

  gl.disable(GL_CULL_FACE);
  gl.depthFunc(GL_LESS);
  size_t texidx = (size_t)(clock() / CLOCKS_PER_SEC / 60.0f) % _textures.size();

  if( mTransparency )
  {
    gl.enable(GL_BLEND);
    gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.depthMask(GL_FALSE);
  }

  const float tcol = mTransparency ? 0.85f : 1.0f;
  ::math::vector_3d col = skies->colorSet[WATER_COLOR_LIGHT] * 0.7f; //! \todo  add variable water color
  ::math::vector_3d col2 = skies->colorSet[WATER_COLOR_DARK] * 0.2f;

  gl.color4f (col.x(), col.y(), col.z(), tcol);
  gl.programLocalParameter4f (GL_FRAGMENT_PROGRAM_ARB, 0, col2.x(), col2.y(), col2.z(), tcol);
  opengl::texture::enable_texture (0);

  _textures[texidx]->bind();

  opengl::texture::enable_texture (1);

  if (mDrawList)
  {
    //! \todo THIS LINE THROWS GL_INVALID_OPERATION! Steff. It donwt do in anymore now. Perhaps because water rendering was called double in maptile::draw()
    mDrawList->render();
  }

  opengl::texture::disable_texture (1);
  opengl::texture::set_active_texture (0);

  gl.color4f(1, 1, 1, 0.4f);
  if (mTransparency)
  {
    gl.depthMask(GL_TRUE);
    gl.disable(GL_BLEND);
  }
  gl.disable(GL_FRAGMENT_PROGRAM_ARB);
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
