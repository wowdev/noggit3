#include "Liquid.h"

#include <algorithm>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "DBC.h"
#include "Log.h"
#include "Shaders.h"
#include "TextureManager.h" // TextureManager, Texture
#include "World.h"


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
  mWaterShader = new BLSShader( "shaders\\pixel\\arbfp1\\psLiquidWater.bls" );
  mMagmaShader = new BLSShader( "shaders\\pixel\\arbfp1\\psLiquidMagma.bls" );
#else
  boost::filesystem::path waterPath("shaders/water.ps");
  boost::filesystem::path fogPath("shaders/waterfog.ps");

  waterPath.make_preferred();
  fogPath.make_preferred();

  FILE *shader = fopen(waterPath.string().c_str(), "r");
  if(!shader)
  {
    LogError << "Unable to open water shader " << waterPath.string() << std::endl;
  }
  else
  {
    char buffer[8192];
    int length=fread(buffer, 1, 8192, shader);
    fclose(shader);
    glGenProgramsARB(1, &waterShader);
    if(!waterShader)
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
      if( !(errorPos==-1)&&(isNative==1) )
      {
        int i, j;
        char localbuffer[256];
        LogError << "Water Shader \"shaders\\water.ps\" Fragment program failed to load \nReason:\n";
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
  if(!shader)
  {
    LogError << "Unable to open water shader " << fogPath.string() << std::endl;
  }
  else
  {
    char buffer[8192];
    int length=fread(buffer, 1, 8192, shader);
    fclose(shader);
    glGenProgramsARB(1, &waterFogShader);
    if(!waterFogShader)
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
      if( !(errorPos==-1)&&(isNative==1) )
      {
        int i, j;
        //const GLubyte *stringy;
        char localbuffer[256];
        LogError << "Water Shader \"shaders/waterfog.ps\" Fragment program failed to load \nReason:\n";
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

#ifndef USEBLSFILES
void enableWaterShader()
{
  if(glIsEnabled(GL_FOG)==GL_TRUE)
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
  else
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
}
#endif

Liquid::Liquid(int x, int y, Vec3D base, float ptilesize)
  : xtiles(x)
  , ytiles(y)
  , pos(base)
  , tilesize(ptilesize)
  , ydir(1.0f)
  , mDrawList(NULL)
{}

Liquid::~Liquid()
{
  if( mDrawList )
  {
    delete mDrawList;
    mDrawList = NULL;
  }

  delTextures();
}

void Liquid::delTextures()
{
  for(std::vector<OpenGL::Texture*>::iterator it = textures.begin(); it != textures.end(); ++it)
  {
    TextureManager::delbyname((*it)->filename());
  }
  textures.clear();
}

void Liquid::initFromWMO(MPQFile* f, const WMOMaterial &mat, bool indoor)
{
  texRepeats = 4.0f;
  ydir = -1.0f;

  initGeometry(f);

  trans = false;

  // tmpflag is the flags value for the last drawn tile
  if (tmpflag & 1) {
    //initTextures<1,30>( "XTEXTURES\\SLIME\\slime.%d.blp" );
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    type = 0;
    texRepeats = 2.0f;
    mTransparency = false;
  }
  else if (tmpflag & 2) {
    //initTextures<1,30>( "XTEXTURES\\LAVA\\lava.%d.blp" );
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    type = 0;
    mTransparency = false;
  }
  else {
    //initTextures<1,30>( "XTEXTURES\\river\\lake_a.%d.blp" );
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    if (indoor) {
      trans = true;
      type = 1;
      col = Vec3D( ( ( mat.col2 & 0xFF0000 ) >> 16 ) / 255.0f, ( ( mat.col2 & 0xFF00 ) >> 8 ) / 255.0f, ( mat.col2 & 0xFF ) / 255.0f);
    } else {
      trans = true;
      type = 2; // outdoor water (...?)
    }
    mTransparency = true;
  }

}


void Liquid::initGeometry(MPQFile* f)
{
  LiquidVertex *map = reinterpret_cast<LiquidVertex*>(f->getPointer());
  unsigned char *flags = reinterpret_cast<unsigned char*>(f->getPointer() + (xtiles+1)*(ytiles+1)*sizeof(LiquidVertex));

  // generate vertices
  Vec3D * lVertices = new Vec3D[(xtiles+1)*(ytiles+1)];
  for (int j=0; j<ytiles+1; j++) {
    for (int i=0; i<xtiles+1; ++i) {
      size_t p = j*(xtiles+1)+i;
      float h = map[p].h;
      if (h > 100000) h = pos.y;
      lVertices[p] = Vec3D(pos.x + tilesize * i, h, pos.z + ydir * tilesize * j);
    }
  }

  mDrawList = new OpenGL::CallList();
  mDrawList->startRecording();

  //! \todo  handle light/dark liquid colors
  
  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);

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

  mDrawList->endRecording();
  if(lVertices)
  {
    delete[] lVertices;
    lVertices = NULL;
  }
}

void Liquid::initFromMH2O(MH2O_Tile &pTileInfo)
{
  if(mDrawList)
  {
    delete mDrawList;
    mDrawList = NULL;
  }

  texRepeats = 4.0f;
  ydir = 1.0f;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID( pTileInfo.mLiquidType );
    initTextures<1,30>( lLiquidTypeRow.getString( LiquidTypeDB::TextureFilenames - 1 ) );
    //initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");

    mLiquidType = lLiquidTypeRow.getInt( LiquidTypeDB::Type );
    mShaderType = lLiquidTypeRow.getInt( LiquidTypeDB::ShaderType );
    //mLiquidType = 0;
    //mShaderType = 1;
    //! \todo  Get texRepeats too.
  }
  catch( ... )
  {
    // Fallback, when there is no information.
    //initTextures<1,30>( "XTEXTURES\\river\\lake_a.%d.blp" );
    initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
    mLiquidType = 0;
    mShaderType = 1;
  }

  mTransparency = mShaderType & 1;

  // generate vertices
  Vec3D lVertices[9][9];
  for( int j = 0; j < 9; ++j )
  {
    for( int i = 0; i < 9; ++i )
    {
      lVertices[j][i] = Vec3D( pos.x + tilesize * i, pTileInfo.mHeightmap[j][i], pos.z + ydir * tilesize * j );
    }
  }

  mDrawList = new OpenGL::CallList();
  mDrawList->startRecording();

  glBegin( GL_QUADS );

  glNormal3f( 0.0f, 1.0f, 0.0f );

  // draw tiles
  for( int j = 0; j < 8; ++j )
  {
    for( int i = 0; i < 8; ++i )
    {
      if(!pTileInfo.mRender[j][i]) continue;

      float c;
      c = pTileInfo.mDepth[j][i];// / 255.0f;
      glMultiTexCoord2f( GL_TEXTURE1, c, c );
      glTexCoord2f( i / texRepeats, j / texRepeats);
      glVertex3fv( lVertices[j][i] );

      c = pTileInfo.mDepth[j][i + 1];// / 255.0f;
      glMultiTexCoord2f( GL_TEXTURE1, c, c );
      glTexCoord2f( ( i + 1 ) / texRepeats, j / texRepeats);
      glVertex3fv( lVertices[j][i + 1] );

      c = pTileInfo.mDepth[j + 1][i + 1];// / 255.0f;
      glMultiTexCoord2f( GL_TEXTURE1, c, c );
      glTexCoord2f( ( i + 1 ) / texRepeats, ( j + 1 ) / texRepeats);
      glVertex3fv( lVertices[j + 1][i + 1] );

      c = pTileInfo.mDepth[j + 1][i];// / 255.0f;
      glMultiTexCoord2f( GL_TEXTURE1, c, c );
      glTexCoord2f( i / texRepeats, ( j + 1 ) / texRepeats);
      glVertex3fv( lVertices[j + 1][i] );
    }
  }


  glEnd();

  mDrawList->endRecording();
}

void Liquid::draw()
{
  glEnable(GL_FRAGMENT_PROGRAM_ARB);
#ifdef USEBLSFILES
  if( type == 2 && mWaterShader->IsOkay() )
    mWaterShader->EnableShader();
  if( type == 0 && mMagmaShader->IsOkay() )
    mMagmaShader->EnableShader();
#else
  enableWaterShader();
#endif

  Vec3D col2;
  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
  size_t texidx = (size_t)(gWorld->animtime / 60.0f) % textures.size();

  //glActiveTexture(GL_TEXTURE0);
  //glDisable(GL_TEXTURE_2D);
  //glBindTexture(GL_TEXTURE_2D, textures[texidx]);

  const float tcol = mTransparency? 0.85f : 1.0f;

  if( mTransparency )
  {
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
  }

#if 0 // 0 - nice water with monotone, 1 - old style water
  if (type==0)
    glColor4f(0.0f,0.0f,0.0f,0.8f);
  else
  {
    if (type==2)
    {
      // dynamic color lookup! ^_^
      col = gWorld->skies->colorSet[WATER_COLOR_LIGHT]*0.3f; //! \todo  add variable water color
      col2 = gWorld->skies->colorSet[WATER_COLOR_DARK];
    }
    glColor4f(col.x, col.y, col.z, tcol);
    glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,col2.x,col2.y,col2.z,tcol);
    //#ifdef USEBLSFILES
    //    glSecondaryColor3f(col2.x,col2.y,col2.z);

    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); //! \todo  check if ARB_texture_env_add is supported? :(
  }
#else 
  col = gWorld->skies->colorSet[WATER_COLOR_LIGHT]*0.7; //! \todo  add variable water color
  col2 = gWorld->skies->colorSet[WATER_COLOR_DARK]*0.2f;

  glColor4f(col.x, col.y, col.z, tcol);
  glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,col2.x,col2.y,col2.z,tcol);
#endif
  OpenGL::Texture::setActiveTexture(0);
  OpenGL::Texture::enableTexture();

  textures[texidx]->bind();

  OpenGL::Texture::setActiveTexture(1);
  OpenGL::Texture::enableTexture();

  if( mDrawList )
  {
    //! \todo THIS LINE THROWS GL_INVALID_OPERATION! Steff. It donwt do in anymore now. Perhaps because water rendering was called double in maptile::draw()
    mDrawList->render();
    CheckForGLError( "Liquid::draw:: after the draw list" );
  }

  OpenGL::Texture::setActiveTexture(1);
  OpenGL::Texture::disableTexture();
  OpenGL::Texture::setActiveTexture(0);

  glColor4f(1,1,1,0.4f);
  if( mTransparency )
  {
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
  glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

template<int pFirst, int pLast>
void Liquid::initTextures( const std::string& pFilename )
{
  textures.clear(); //! \todo this assumes that the same textures are used af befor!
  for( int i = pFirst; i <= pLast; ++i )
  {
    std::string filename(boost::str(boost::format(pFilename) % i));
    textures.push_back(TextureManager::newTexture(filename));
  }
}



