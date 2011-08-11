#include "Video.h"

#include <SDL.h>
#include <string>

#include "Settings.h"
#include "Log.h"

//0 - GL_ALPHA_TEST
//1 - GL_BLEND
//2 - GL_COLOR_MATERIAL
//3 - GL_CULL_FACE
//4 - GL_DEPTH_TEST
//5 - GL_FOG
//6 - GL_FRAGMENT_PROGRAM_ARB
//7 - GL_LIGHTING
//8 - GL_LINE_SMOOTH
//9/10 - GL_TEXTURE_2D
//11 - GL_TEXTURE_GEN_S
//12 - GL_TEXTURE_GEN_T
GLboolean  GLSettings[13];
void SaveGLSettings()
{
  GLSettings[0]=glIsEnabled(GL_ALPHA_TEST);
  GLSettings[1]=glIsEnabled(GL_BLEND);
  GLSettings[2]=glIsEnabled(GL_COLOR_MATERIAL);
  GLSettings[3]=glIsEnabled(GL_CULL_FACE);
  GLSettings[4]=glIsEnabled(GL_DEPTH_TEST);
  GLSettings[5]=glIsEnabled(GL_FOG);
  GLSettings[6]=glIsEnabled(GL_FRAGMENT_PROGRAM_ARB);
  GLSettings[7]=glIsEnabled(GL_LIGHTING);
  GLSettings[8]=glIsEnabled(GL_LINE_SMOOTH);
  glActiveTexture(GL_TEXTURE1);
  GLSettings[10]=glIsEnabled(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  GLSettings[9]=glIsEnabled(GL_TEXTURE_2D);
  GLSettings[11]=glIsEnabled(GL_TEXTURE_GEN_S);
  GLSettings[12]=glIsEnabled(GL_TEXTURE_GEN_T);
}

void compareSetting(GLenum Setting, GLboolean Cur)
{
  if(glIsEnabled(Setting)!=Cur)
  {
    if(Cur==GL_TRUE)
      glEnable(Setting);
    else
      glDisable(Setting);
  }
}

void LoadGLSettings()
{
  compareSetting(GL_ALPHA_TEST,GLSettings[0]);
  compareSetting(GL_BLEND,GLSettings[1]);
  compareSetting(GL_COLOR_MATERIAL,GLSettings[2]);
  compareSetting(GL_CULL_FACE,GLSettings[3]);
  compareSetting(GL_DEPTH_TEST,GLSettings[4]);
  compareSetting(GL_FOG,GLSettings[5]);
  compareSetting(GL_FRAGMENT_PROGRAM_ARB,GLSettings[6]);
  compareSetting(GL_LIGHTING,GLSettings[7]);
  compareSetting(GL_LINE_SMOOTH,GLSettings[8]);
  glActiveTexture(GL_TEXTURE1);
  compareSetting(GL_TEXTURE_2D,GLSettings[10]);
  glActiveTexture(GL_TEXTURE0);
  compareSetting(GL_TEXTURE_2D,GLSettings[9]);
  compareSetting(GL_TEXTURE_GEN_S,GLSettings[11]);
  compareSetting(GL_TEXTURE_GEN_T,GLSettings[12]);
}

Video video;


int Video::xres() const
{
  return _xres;
}
int Video::yres() const
{
  return _yres;
}
float Video::ratio() const
{
  return _ratio;
}
bool Video::fullscreen() const
{
  return _fullscreen;
}
float Video::fov() const
{
  return _fov;
}
float Video::nearclip() const
{
  return _nearclip;
}
float Video::farclip() const
{
  return _farclip;
}

void Video::fov( float fov_ )
{
  _fov = fov_;
}
void Video::nearclip( float nearclip_ )
{
  _nearclip = nearclip_;
}
void Video::farclip( float farclip_ )
{
  _farclip = farclip_;
}

void Video::updateProjectionMatrix()
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glViewport( 0.0f, 0.0f, xres(), yres() );
  gluPerspective( fov(), ratio(), nearclip(), farclip() );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void Video::resize( int xres_, int yres_ )
{
  _xres = xres_;
  _yres = yres_;
  _ratio = static_cast<float>( xres() ) / static_cast<float>( yres() );
  
  LogDebug << "resize(" << xres() << ", " << yres() << ");" << std::endl;

  SDL_Rect rect = { 0, 0, xres(), yres() };
  SDL_SetClipRect( _primary, &rect );

  updateProjectionMatrix();
}

bool Video::init( int xres_, int yres_, bool fullscreen_ )
{
  _xres = xres_;
  _yres = yres_;
  _ratio = static_cast<float>( xres() ) / static_cast<float>( yres() );
  
  _fov = 45.0f;
  _nearclip = 1.0f;
  _farclip = Settings::getInstance()->FarZ;
  
  _fullscreen = fullscreen_;
  
  if( SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO ) )
  {
    LogError << "SDL: " << SDL_GetError() << std::endl;
    exit( 1 );
  }
  
  int flags = SDL_OPENGL | SDL_HWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE;
  if( fullscreen() )
  {
    flags |= SDL_FULLSCREEN;
  }

  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
  
  _primary = SDL_SetVideoMode( _xres, _yres, 0, flags );

  if( !_primary ) 
  {
    LogError << "SDL: " << SDL_GetError() << std::endl;
    exit( 1 );
  }

  GLenum err = glewInit();
  if( GLEW_OK != err )
  {
    LogError << "GLEW: " << glewGetErrorString( err ) << std::endl;
    return false;
  }

  glViewport( 0.0f, 0.0f, xres(), yres() );

  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_NORMAL_ARRAY );
  glEnableClientState( GL_TEXTURE_COORD_ARRAY );

  mSupportCompression = GLEW_ARB_texture_compression;
  mSupportShaders = GLEW_ARB_vertex_program && GLEW_ARB_fragment_program;

  LogDebug << "GL: Version: " << glGetString( GL_VERSION ) << std::endl;
  LogDebug << "GL: Vendor: " << glGetString( GL_VENDOR ) << std::endl;
  LogDebug << "GL: Renderer: " << glGetString( GL_RENDERER ) << std::endl;
  
  return mSupportCompression;
}

void Video::close()
{
  SDL_FreeSurface( _primary );
  SDL_Quit();
}

void Video::flip() const
{
  SDL_GL_SwapBuffers();
}

void Video::clearScreen() const
{
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Video::set3D() const
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( fov(), ratio(), nearclip(), farclip() );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void Video::set3D_select() const
{
  glMatrixMode( GL_PROJECTION );
  gluPerspective( fov(), ratio(), nearclip(), farclip() );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void Video::set2D() const
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0.0f, xres(), yres(), 0.0f, -1.0f, 1.0f );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void Video::setTileMode() const
{
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( -2.0f * ratio(), 2.0f * ratio(), 2.0f, -2.0f, -100.0f, 300.0f );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}

void CheckForGLError( const std::string& pLocation )
{
  int ErrorNum = glGetError();
  while( ErrorNum )
  {
    switch( ErrorNum )
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



#include <stdint.h>
//! \todo Cross-platform syntax for packed structs.
#pragma pack(push,1)
struct BLPHeader 
{
  int32_t magix;
  int32_t version;
  char attr_0_compression;
  char attr_1_alphadepth;
  char attr_2_alphatype;
  char attr_3_mipmaplevels;
  int32_t resx;
  int32_t resy;
  int32_t offsets[16];
  int32_t sizes[16];
};
#pragma pack(pop)

#include <boost/thread.hpp>
#include "MPQ.h"

namespace OpenGL
{
  CallList::CallList()
  {
    list = glGenLists( 1 );
  }
  CallList::~CallList()
  {
    glDeleteLists( list, 1 );
  }

  void CallList::startRecording( ModeEnum mode )
  {
    glNewList( list, mode );
  }
  void CallList::endRecording()
  {
    glEndList();
  }
  void CallList::render()
  {
    glCallList( list );
  }
    
  Texture::Texture()
  : ManagedItem( )
  , _width( 0 )
  , _height( 0 )
  , _id( 0 )
  , _filename( "" )
  {
    glGenTextures( 1, &_id );
  }
  
  Texture::~Texture()
  {
    invalidate();
  }
  
  void Texture::invalidate()
  {
    glDeleteTextures( 1, &_id );
    _id = 0;
  }
  
  void Texture::bind() const
  {
    glBindTexture( GL_TEXTURE_2D, _id );
  }
  
  void Texture::enableTexture()
  {
    glEnable( GL_TEXTURE_2D );
  }
  void Texture::disableTexture()
  {
    glDisable( GL_TEXTURE_2D );
  }
  void Texture::setActiveTexture( size_t num )
  {
    glActiveTexture( GL_TEXTURE0 + num );
  }
  
  void Texture::loadFromUncompressedData( BLPHeader* lHeader, char* lData )
  {
    unsigned int * pal = reinterpret_cast<unsigned int*>( lData + sizeof( BLPHeader ) );
  
    unsigned char *buf;
    unsigned int *buf2 = new unsigned int[_width*_height];
    unsigned int *p;
    unsigned char *c, *a;
  
    int alphabits = lHeader->attr_1_alphadepth;
    bool hasalpha = alphabits != 0;
  
    for (int i=0; i<16; ++i)
    {
      _width = std::max( 1, _width );
      _height = std::max( 1, _height );
      
      if (lHeader->offsets[i] && lHeader->sizes[i])
      {
        buf = reinterpret_cast<unsigned char*>( &lData[lHeader->offsets[i]] );
  
        int cnt = 0;
        p = buf2;
        c = buf;
        a = buf + _width*_height;
        for (int y=0; y<_height; y++)
        {
          for (int x=0; x<_width; x++)
          {
            unsigned int k = pal[*c++];
            k = ( ( k & 0x00FF0000 ) >> 16 ) | ( ( k & 0x0000FF00 ) ) | ( ( k & 0x000000FF ) << 16 );
            int alpha = 0xFF;
            if (hasalpha) 
            {
              if (alphabits == 8) 
              {
                alpha = (*a++);
              } 
              else if (alphabits == 1)
              {
                alpha = (*a & (1 << cnt++)) ? 0xff : 0;
                if (cnt == 8) 
                {
                  cnt = 0;
                  a++;
                }
              }
            }
  
            k |= alpha << 24;
            *p++ = k;
          }
        }
  
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf2);
  
      }
      else
      {
        return;
      }
      
      _width >>= 1;
      _height >>= 1;
    }
  
    delete[] buf2;
    delete[] buf;
  }
  
  void Texture::loadFromCompressedData( BLPHeader* lHeader, char* lData )
  {
    //                         0 (0000) & 3 == 0                1 (0001) & 3 == 1                    7 (0111) & 3 == 3
    const int alphatypes[] = { GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT };
    const int blocksizes[] = { 8,                               16,                               0, 16 };
    
    int lTempAlphatype = lHeader->attr_2_alphatype & 3;
    GLint format = alphatypes[lTempAlphatype];
    int blocksize = blocksizes[lTempAlphatype];
    format = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? ( lHeader->attr_1_alphadepth == 1 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT ) : format;

    // do every mipmap level
    for( int i = 0; i < 16; ++i ) 
    {
      _width = std::max( 1, _width );
      _height = std::max( 1, _height );

      if( lHeader->offsets[i] && lHeader->sizes[i] ) 
      {
        glCompressedTexImage2D( GL_TEXTURE_2D, i, format, _width, _height, 0, ( (_width + 3) / 4) * ( (_height + 3 ) / 4 ) * blocksize, reinterpret_cast<char*>( lData + lHeader->offsets[i] ) );
      }
      else
      { 
        return;
      }

      _width >>= 1;
      _height >>= 1;
    }
  }
  
  const std::string& Texture::filename()
  {
    return _filename;
  }
  
  void Texture::loadFromBLP( const std::string& filenameArg )
  {
    //! \todo Unload if there already is a model loaded?
    _filename = filenameArg;
    
    bind();
    
    MPQFile f( _filename );
    if( f.isEof() ) 
    {
      invalidate();
      return;
    }
  
    char* lData = f.getPointer();
    BLPHeader* lHeader = reinterpret_cast<BLPHeader*>( lData );
    _width = lHeader->resx;
    _height = lHeader->resy;
  
    if( lHeader->attr_0_compression == 1 )
    {
      loadFromUncompressedData( lHeader, lData );
    }
    else if( lHeader->attr_0_compression == 2 )
    {
      loadFromCompressedData( lHeader, lData );
    }
  
    f.close();
  
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }
}
