// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Shaders.h>

#include <string>
#include <string.h>
#include <stdio.h>

#include <noggit/Log.h>
#include <noggit/mpq/file.h>

#include <opengl/context.hpp>

#ifdef USEBLSSHADER
BLSShader::BLSShader( const QString & pFilename )
{
  mOkay = false;
  if( !noggit::mpq::file::exists( pFilename ) )
  {
    LogError << "Failed to get file for shader \"" << pFilename.toStdString() << "\"." << std::endl;
    return;
  }

  int magix;
  int length;
  char * buffer;

  noggit::mpq::file lShader( pFilename );
  lShader.read( &magix, sizeof( int ) );
  lShader.seek( 0x18 );
  lShader.read( &length, sizeof( int ) );
  length--;
  buffer = new char[length];
  lShader.read( buffer, length );

  for( ; buffer[length] <= 0; length-- ) { }

  lShader.seek( 0x1C );
  buffer = new char[length];
  lShader.read( buffer, length );

  mProgramType = magix == 'GXPS' ? GL_FRAGMENT_PROGRAM_ARB : ( 'GXVS' ? GL_VERTEX_PROGRAM_ARB : -1 );

  gl.genPrograms( 1, &mShader );
  if( !mShader )
  {
    LogError << "Failed to get program ID for shader \"" << pFilename.toStdString() << "\"." << std::endl;
  }
  else
  {
    GLint errorPos, isNative;

    gl.bindProgram( mProgramType, mShader );
    gl.programString( mProgramType, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer );
    gl.getIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errorPos );

    gl.getProgramiv( mProgramType, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative );
    if( !( errorPos == -1 && isNative == 1 ) )
    {
      LogError << "Shader program \"" << pFilename.toStdString() << "\" failed to load. Reason:" << std::endl;
      if( isNative == 0 )
        LogError << "\t\"This fragment program exceeded the limit.\"" << std::endl;

      const GLubyte *stringy;
      stringy = gl.getString(GL_PROGRAM_ERROR_STRING_ARB);      //This is only available in ARB
      if( stringy )
        LogError << "\t\"" << reinterpret_cast<const char*>( stringy ) << "\"" << std::endl;

      int j = 0;
      char localbuffer[256];
      for( int i = errorPos; i < length && j < 128 ; ++i, j++ )
      {
        localbuffer[j] = buffer[i];
      }
      localbuffer[j] = 0;
      if( localbuffer[0] )
        LogError << "\tDump:\"" << localbuffer<< "\"" << std::endl;
    }
    else
      mOkay = true;
  }
}
#endif


ShaderPair *terrainShaders[4]={nullptr,nullptr,nullptr,nullptr}, *wmoShader=nullptr, *waterShaders[1]={nullptr};

void reloadShaders()
{
  for (int i=0; i<4; ++i)
  {
    delete terrainShaders[i];
    terrainShaders[i] = nullptr;
  }

  delete wmoShader;
  wmoShader = nullptr;
  delete waterShaders[0];
  waterShaders[0] = nullptr;

  terrainShaders[0] = new ShaderPair(0, "shaders/terrain1.fs", true);
  terrainShaders[1] = new ShaderPair(0, "shaders/terrain2.fs", true);
  terrainShaders[2] = new ShaderPair(0, "shaders/terrain3.fs", true);
  terrainShaders[3] = new ShaderPair(0, "shaders/terrain4.fs", true);
  wmoShader = new ShaderPair(0, "shaders/wmospecular.fs", true);
  waterShaders[0] = new ShaderPair(0, "shaders/wateroutdoor.fs", true);
}

Shader::Shader(GLenum _target, const char *program, bool fromFile):id(0),target(_target)
{
  if (!program || !strlen(program)) {
    ok = true;
    return;
  }

  const char *progtext;
  if (fromFile) {
    char *buf;
    FILE *f = fopen(program, "rb");
    if (!f) {
      ok = false;
      return;
    }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = new char[len+1];
    progtext = buf;
    fread(buf, len, 1, f);
    buf[len]=0;
    fclose(f);
    //gLog("Len: %d\nShader text:\n[%s]\n",len,progtext);
    if( buf )
    {
      delete[] buf;
      buf = nullptr;
    }
  } else progtext = program;

  gl.genPrograms(1, &id);
  gl.bindProgram(target, id);
  gl.programString(target, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(progtext), progtext);
  ok = true;
}

Shader::~Shader()
{
  if (ok && id) gl.deletePrograms(1, &id);
}

void Shader::bind()
{
  gl.bindProgram(target, id);
  gl.enable(target);
}

void Shader::unbind()
{
  gl.disable(target);
}

ShaderPair::ShaderPair(const char *vprog, const char *fprog, bool fromFile)
{
  if (vprog && strlen(vprog)) {
    vertex = new Shader(GL_VERTEX_PROGRAM_ARB, vprog, fromFile);
    if (vertex && !vertex->ok) {
      delete vertex;
      vertex = nullptr;
    }
  } else vertex = nullptr;
  if (fprog && strlen(fprog)) {
    fragment = new Shader(GL_FRAGMENT_PROGRAM_ARB, fprog, fromFile);
    if (fragment && !fragment->ok) {
      delete fragment;
      fragment = nullptr;
    }
  } else fragment = nullptr;
}

void ShaderPair::bind()
{
  if (vertex)
  {
    vertex->bind();
  } else {
    gl.disable(GL_VERTEX_PROGRAM_ARB);
  }
  if (fragment) {
    fragment->bind();
  } else {
    gl.disable(GL_FRAGMENT_PROGRAM_ARB);
  }
}

void ShaderPair::unbind()
{
  if (vertex) vertex->unbind();
  if (fragment) fragment->unbind();
}
