// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/vector_2d.hpp>
#include <noggit/TextureManager.h>
#include <noggit/Log.h> // LogDebug
#include <opengl/context.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <QtCore/QString>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFramebufferObjectFormat>
#include <QtGui/QPixmap>
#include <QtOpenGL/QGLPixelBuffer>

#include <algorithm>

std::atomic<int> blp_texture::blp_tex_counter = {0};

decltype (TextureManager::_) TextureManager::_;

void TextureManager::report()
{
  std::string output = "Still in the Texture manager:\n";
  _.apply ( [&] (std::string const& key, blp_texture const&)
            {
              output += " - " + key + "\n";
            }
          );
  LogDebug << output;
}

#include <cstdint>
//! \todo Cross-platform syntax for packed structs.
#pragma pack(push,1)
struct BLPHeader
{
  int32_t magix;
  int32_t version;
  uint8_t attr_0_compression;
  uint8_t attr_1_alphadepth;
  uint8_t attr_2_alphatype;
  uint8_t attr_3_mipmaplevels;
  int32_t resx;
  int32_t resy;
  int32_t offsets[16];
  int32_t sizes[16];
};
#pragma pack(pop)

#include <boost/thread.hpp>
#include <noggit/MPQ.h>

void blp_texture::bind()
{
  opengl::texture::bind();

  if (!finished)
  {
    return;
  }

  if (!_uploaded)
  {
    upload();
  }
}

void blp_texture::upload()
{
  if (_uploaded)
  {
    return;
  }

  int width = _width, height = _height;

  if (!_compression_format)
  {
    for (int i = 0; i < _data.size(); ++i)
    {
      gl.texImage2D(GL_TEXTURE_2D, i, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data[i].data());

      width = std::max(width >> 1, 1);
      height = std::max(height >> 1, 1);
    }

    _data.clear();
  }
  else
  {
    for (int i = 0; i < _compressed_data.size(); ++i)
    {
      gl.compressedTexImage2D(GL_TEXTURE_2D, i, _compression_format.get(), width, height, 0, _compressed_data[i].size(), _compressed_data[i].data());

      width = std::max(width >> 1, 1);
      height = std::max(height >> 1, 1);
    }

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, _compressed_data.size() - 1);
    _compressed_data.clear();
  }

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  _uploaded = true;
}

void blp_texture::loadFromUncompressedData(BLPHeader const* lHeader, char const* lData)
{
  unsigned int const* pal = reinterpret_cast<unsigned int const*>(lData + sizeof(BLPHeader));

  unsigned char const* buf;
  unsigned int *p;
  unsigned char const* c;
  unsigned char const* a;

  int alphabits = lHeader->attr_1_alphadepth;
  bool hasalpha = alphabits != 0;

  int width = _width, height = _height;

  for (int i = 0; i<16; ++i)
  {
    width = std::max(1, width);
    height = std::max(1, height);

    if (lHeader->offsets[i] > 0 && lHeader->sizes[i] > 0)
    {
      buf = reinterpret_cast<unsigned char const*>(&lData[lHeader->offsets[i]]);

      std::vector<uint32_t> data(lHeader->sizes[i]);

      int cnt = 0;
      p = data.data();
      c = buf;
      a = buf + width*height;
      for (int y = 0; y<height; y++)
      {
        for (int x = 0; x<width; x++)
        {
          unsigned int k = pal[*c++];
          k = ((k & 0x00FF0000) >> 16) | ((k & 0x0000FF00)) | ((k & 0x000000FF) << 16);
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

      _data[i] = data;
    }
    else
    {
      return;
    }

    width >>= 1;
    height >>= 1;
  }
}

void blp_texture::loadFromCompressedData(BLPHeader const* lHeader, char const* lData)
{
  //                         0 (0000) & 3 == 0                1 (0001) & 3 == 1                    7 (0111) & 3 == 3
  const int alphatypes[] = { GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT };
  const int blocksizes[] = { 8, 16, 0, 16 };

  int alpha_type = lHeader->attr_2_alphatype & 3;
  GLint format = alphatypes[alpha_type];
  _compression_format = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? (lHeader->attr_1_alphadepth == 1 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT) : format;

  int width = _width, height = _height;

  for (int i = 0; i < 16; ++i)
  {
    if (lHeader->sizes[i] <= 0 || lHeader->offsets[i] <= 0)
    {
      return;
    }

    // make sure the vector is of the right size, blizzard seems to fuck those up for some small mipmaps
    int size = std::floor((width + 3) / 4) * std::floor((height + 3) / 4) * blocksizes[alpha_type];

    if (size < lHeader->sizes[i])
    {
      LogDebug << "mipmap size mismatch in '" << filename << "'" << std::endl;
      return;
    }

    _compressed_data[i].resize(size);

    char const* start = lData + lHeader->offsets[i];
    std::copy(start, start + lHeader->sizes[i], _compressed_data[i].begin());

    width = std::max(width >> 1, 1);
    height = std::max(height >> 1, 1);
  }
}

blp_texture::blp_texture(const std::string& filenameArg)
  : AsyncObject(filenameArg)
  , public_id(blp_tex_counter++)
{
}

void blp_texture::finishLoading()
{
  bool exists = MPQFile::exists(filename);
  if (!exists)
  {
    LogError << "file not found: '" << filename << "'" << std::endl;
  }

  MPQFile f(exists ? filename : "textures/shanecube.blp");
  if (f.isEof())
  {
    finished = true;
    throw std::runtime_error ("File " + filename + " does not exists");
  }

  char const* lData = f.getPointer();
  BLPHeader const* lHeader = reinterpret_cast<BLPHeader const*>(lData);
  _width = lHeader->resx;
  _height = lHeader->resy;

  if (lHeader->attr_0_compression == 1)
  {
    loadFromUncompressedData(lHeader, lData);
  }
  else if (lHeader->attr_0_compression == 2)
  {
    loadFromCompressedData(lHeader, lData);
  }
  else
  {
    finished = true;
    throw std::logic_error ("unimplemented BLP colorEncoding");
  }

  f.close();
  finished = true;
  _state_changed.notify_all();
}

namespace noggit
{
  QPixmap render_blp_to_pixmap ( std::string const& blp_filename
                               , int width
                               , int height
                               )
  {
    opengl::context::save_current_context const context_save (::gl);

    QOpenGLContext context;
    context.create();

    QOpenGLFramebufferObjectFormat fmt;
    fmt.setSamples(1);
    fmt.setInternalTextureFormat(GL_RGBA8);

    QOffscreenSurface surface;
    surface.create();

    context.makeCurrent(&surface);

    opengl::context::scoped_setter const context_set (::gl, &context);

    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> cull;
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> depth;

    opengl::scoped::deferred_upload_vertex_arrays<1> vao;
    vao.upload();
    opengl::scoped::deferred_upload_buffers<3> buffers;
    buffers.upload();
    GLuint const& indices_vbo = buffers[0];
    GLuint const& vertices_vbo = buffers[1];
    GLuint const& texcoords_vbo = buffers[2];

    opengl::texture::set_active_texture(0);
    blp_texture texture(blp_filename);
    texture.finishLoading();

    width = width == -1 ? texture.width() : width;
    height = height == -1 ? texture.height() : height;

    float h = static_cast<float>(height);
    float w = static_cast<float>(width);
    float half_h = h * 0.5f;
    float half_w = w * 0.5f;

    std::vector<math::vector_2d> vertices =
    {
       {-half_w, -half_h}
      ,{-half_w, half_h}
      ,{ half_w, half_h}
      ,{ half_w, -half_h}
    };
    std::vector<math::vector_2d> texcoords =
    {
       {0.f, 0.f}
      ,{0.f, h}
      ,{w, h}
      ,{w, 0.f}
    };
    std::vector<std::uint16_t> indices = {0,1,2, 2,3,0};

    gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(vertices_vbo, vertices, GL_STATIC_DRAW);
    gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(texcoords_vbo, texcoords, GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(indices_vbo, indices, GL_STATIC_DRAW);

    QOpenGLFramebufferObject pixel_buffer(width, height, fmt);
    pixel_buffer.bind();

    gl.viewport(0, 0, w, h);
    gl.clearColor(.0f, .0f, .0f, 1.f);
    gl.clear(GL_COLOR_BUFFER_BIT);

    opengl::program program
    (
      {
        {
          GL_VERTEX_SHADER, R"code(
#version 330 core

in vec4 position;
in vec2 tex_coord;
out vec2 f_tex_coord;

void main()
{
f_tex_coord = vec2(tex_coord.x, -tex_coord.y);
gl_Position = position;
}
)code"
        },
        {
          GL_FRAGMENT_SHADER,
          R"code(
#version 330 core

uniform sampler2D tex;

in vec2 f_tex_coord;

layout(location = 0) out vec4 out_color;

void main()
{
out_color = vec4(texture(tex, f_tex_coord/2.f + vec2(0.5)).rgb, 1.);
}
)code"
        }
      }
    );

    opengl::scoped::use_program shader (program);

    shader.uniform("tex", 0);
    texture.bind();

    opengl::scoped::vao_binder const _ (vao[0]);

    shader.attrib(_, "position", vertices_vbo, 2, GL_FLOAT, GL_FALSE, 0, 0);
    shader.attrib(_, "tex_coord", texcoords_vbo, 2, GL_FLOAT, GL_FALSE, 0, 0);
    opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> indices_binder(indices_vbo);

    gl.drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices_vbo);

    QPixmap pixmap (QPixmap::fromImage (pixel_buffer.toImage()));

    pixel_buffer.release();

    if (pixmap.isNull())
    {
      throw std::runtime_error
        ("failed rendering " + blp_filename + " to pixmap");
    }

    return pixmap;
  }
}

scoped_blp_texture_reference::scoped_blp_texture_reference (std::string const& filename)
  : _blp_texture (TextureManager::_.emplace (filename))
  , _blp_id(_blp_texture->public_id)
{}

scoped_blp_texture_reference::scoped_blp_texture_reference (scoped_blp_texture_reference const& other)
  : _blp_texture (other._blp_texture ? TextureManager::_.emplace (other._blp_texture->filename) : nullptr)
  , _blp_id(other._blp_id)
{}

void scoped_blp_texture_reference::Deleter::operator() (blp_texture* texture) const
{
  TextureManager::_.erase (texture->filename);
}

blp_texture* scoped_blp_texture_reference::operator->() const
{
  return _blp_texture.get();
}
blp_texture* scoped_blp_texture_reference::get() const
{
  return _blp_texture.get();
}

bool scoped_blp_texture_reference::operator== (scoped_blp_texture_reference const& other) const
{
  return std::tie (_blp_texture) == std::tie (other._blp_texture);
}
