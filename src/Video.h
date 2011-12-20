#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <stack>

class Video;

#include <gl/glew.h>

class Video
{
public:
  bool init(int xres_, int yres_);

  void close();

  void flip() const {}
  void clearScreen() const;
  void set3D() const;
  void set3D_select() const;
  void set2D() const;
  void setTileMode() const;
  void resize(int w, int h);

  inline const int& xres() const
  {
    return _xres;
  }
  inline const int& yres() const
  {
    return _yres;
  }
  inline const float& ratio() const
  {
    return _ratio;
  }
  inline const float& fov() const
  {
    return _fov;
  }
  inline const float& nearclip() const
  {
    return _nearclip;
  }
  inline const float& farclip() const
  {
    return _farclip;
  }

  void fov( const float& fov_ )
  {
    _fov = fov_;
  }
   void nearclip( const float& nearclip_ )
  {
    _nearclip = nearclip_;
  }
   void farclip( const float& farclip_ )
  {
    _farclip = farclip_;
  }

  void updateProjectionMatrix();

  /// is * supported:
  bool mSupportShaders;
  bool mSupportCompression;

private:
  int _xres;
  int _yres;
  float _ratio;

  float _fov;
  float _nearclip;
  float _farclip;
};

#include "Manager.h" // ManagedItem

struct BLPHeader;

namespace OpenGL
{
  class SettingsSaver
  {
  private:
    struct GLSettings
    {
      GLboolean alphaTesting;
      GLboolean blend;
      GLboolean colorMaterial;
      GLboolean cullFace;
      GLboolean depthTest;
      GLboolean fog;
      GLboolean fragmentProgram;
      GLboolean lighting;
      GLboolean lineSmooth;
      GLboolean texture0;
      GLboolean texture1;
      GLboolean textureGenS;
      GLboolean textureGenT;
    };

    static std::stack<GLSettings> _savedSettings;

  public:
    static void save();
    static void restore();
  };

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
    static void enableTexture( size_t num );
    static void disableTexture();
    static void disableTexture( size_t num );
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
