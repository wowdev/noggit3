#ifndef VIDEO_H
#define VIDEO_H

#include <string>

class Video;

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
//#include <SDL/SDL.h>

class SDL_Surface;

void SaveGLSettings();
void LoadGLSettings();

class Video
{
  int status;
  int flags;

  SDL_Surface *primary;

public:
  bool fullscreen;

  bool init(int xres, int yres, bool fullscreen);

  void close();

  void flip() const;
  void clearScreen() const;
  void set3D() const;
  void set3D_select() const;
  void set2D() const;
  void setTileMode() const;
  void resize(int w, int h);
    
  int xres, yres;
  int origX, origY;
  float ratio;

  /// is * supported:
  bool mSupportShaders;
  bool mSupportCompression;
};

#include "Manager.h" // ManagedItem

struct BLPHeader;

namespace OpenGL
{  
  class CallList
  {
  public:
    CallList();
    ~CallList();
    
    typedef GLuint ModeEnum;

    void startRecording(ModeEnum mode = GL_COMPILE);
    void endRecording();
    void render();
    
  private:
    typedef GLuint InternalRepresentation;
    
    InternalRepresentation list;
  };
  
  class Texture : public ManagedItem
  {
  public:
    typedef GLuint InternalRepresentation;
    
    Texture();
    ~Texture();
    
    void invalidate();
    
    void loadFromBLP( const std::string& filename );
    void loadFromUncompressedData( BLPHeader* lHeader, char* lData );
    void loadFromCompressedData( BLPHeader* lHeader, char* lData );
    
    void bind() const;
    
    static void enableTexture();
    static void disableTexture();
    static void setActiveTexture( size_t num = 0 );
    
    const std::string& filename();
    
  private:
    int _width;
    int _height;
    InternalRepresentation _id;
    std::string _filename;
  };
  
  typedef GLuint Shader;
  typedef GLuint Light;
}

extern Video video;

bool isExtensionSupported(const char *search);
void CheckForGLError( const std::string& pLocation );

#endif
