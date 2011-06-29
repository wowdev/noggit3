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

namespace OpenGL
{  
  class CallList
  {
    GLuint list;
  public:
    CallList();
    ~CallList();

    void startRecording(GLuint mode = GL_COMPILE);
    void endRecording();
    void render();
  };
  
  class Texture : public ManagedItem
  {
  public: //! \todo make private again and fix friends.
    int w,h;
    GLuint id;
    
    explicit Texture(const std::string& pname);
    const GLuint getId() const;
    void render() const;
    
    static void enableTexture();
    static void disableTexture();
    static void setActiveTexture( size_t num = 0 );
  };
}

extern Video video;

//GLuint loadTGA(const char *filename, bool mipmaps);
bool isExtensionSupported(const char *search);
void CheckForGLError( const std::string& pLocation );

#endif
