// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Texture.h>

#include <string>

#include <noggit/TextureManager.h> // TextureManager
#include <noggit/Video.h> // Texture
#include <opengl/scoped.hpp>

UITexture::UITexture(float xPos, float yPos, float w, float h, const std::string& tex)
  : UIFrame(xPos, yPos, w, h)
  , texture(tex)
  , highlight(false)
  , clickFunc(nullptr)
  , id(0)
{
}

void UITexture::setTexture(scoped_blp_texture_reference const& tex)
{
  texture = tex;
}

void UITexture::setTexture(const std::string& textureFilename)
{
  texture = textureFilename;
}

scoped_blp_texture_reference const& UITexture::getTexture()
{
  return texture;
}

void UITexture::render() const
{
  opengl::scoped::matrix_pusher const matrix;
  gl.translatef(x(), y(), 0.0f);

  gl.color3f(1.0f, 1.0f, 1.0f);

  opengl::texture::set_active_texture();
  opengl::texture::enable_texture();

  texture->bind();

  gl.begin(GL_TRIANGLE_STRIP);
  gl.texCoord2f(0.0f, 0.0f);
  gl.vertex2f(0.0f, 0.0f);
  gl.texCoord2f(1.0f, 0.0f);
  gl.vertex2f(width(), 0.0f);
  gl.texCoord2f(0.0f, 1.0f);
  gl.vertex2f(0.0f, height());
  gl.texCoord2f(1.0f, 1.0f);
  gl.vertex2f(width(), height());
  gl.end();

  opengl::texture::disable_texture();

  if (highlight)
  {
    gl.color3f(1.0f, 0.0f, 0.0f);
    gl.begin(GL_LINE_LOOP);
    gl.vertex2f(-1.0f, 0.0f);
    gl.vertex2f(width(), 0.0f);
    gl.vertex2f(width(), height());
    gl.vertex2f(-1.0f, height());
    gl.end();
  }
}

UIFrame *UITexture::processLeftClick(float /*mx*/, float /*my*/)
{
  if (clickFunc)
  {
    clickFunc();
    return this;
  }
  return 0;
}

void UITexture::setClickFunc (std::function<void()> fun)
{
  clickFunc = fun;
}
