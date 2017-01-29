// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <math/vector_4d.hpp>
#include <opengl/context.hpp>
#include <opengl/texture.hpp>

#include <stack>
#include <string>

class Video;

struct SDL_Surface;

class Video
{
public:
  bool init(int xres_, int yres_, bool fullscreen_, bool doAntiAliasing_);

  void close();

  void flip() const;
  void clearScreen() const;
  void set3D() const;
  void set2D() const;
  void setTileMode() const;
  void resize(int w, int h);

  int xres() const
  {
    return _xres;
  }
  int yres() const
  {
    return _yres;
  }
  float ratio() const
  {
    return _ratio;
  }
  bool fullscreen() const
  {
    return _fullscreen;
  }
  bool doAntiAliasing() const
  {
    return _doAntiAliasing;
  }
  math::degrees fov() const
  {
    return _fov;
  }
  float nearclip() const
  {
    return _nearclip;
  }
  float farclip() const
  {
    return _farclip;
  }

  void doAntiAliasing(bool doAntiAliasing_)
  {
    _doAntiAliasing = doAntiAliasing_;
  }
  void fov(math::degrees fov_)
  {
    _fov = fov_;
  }
  void nearclip(float nearclip_)
  {
    _nearclip = nearclip_;
  }
  void farclip(float farclip_)
  {
    _farclip = farclip_;
  }

  math::vector_4d normalized_device_coords (int x, int y) const;

  /// is * supported:
  bool mSupportShaders;
  bool mSupportCompression;

private:
  int _xres;
  int _yres;
  float _ratio;

  math::degrees _fov = math::degrees (45.0f);
  float _nearclip;
  float _farclip;

  bool _fullscreen;
  bool _doAntiAliasing;

  SDL_Surface* _primary;
};

struct BLPHeader;

namespace OpenGL
{
  class Texture : public opengl::texture
  {
  public:
    Texture();

    void loadFromBLP(const std::string& filename);
    void loadFromUncompressedData(BLPHeader* lHeader, char* lData);
    void loadFromCompressedData(BLPHeader* lHeader, char* lData);

    const std::string& filename();

  private:
    int _width;
    int _height;
    std::string _filename;
  };

  typedef GLuint Shader;
  typedef GLuint Light;
}

extern Video video;

//bool isExtensionSupported(const char *search);
