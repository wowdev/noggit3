// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ToolbarIcon.h>

#include <string>

#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/Video.h> // gl*
#include <opengl/scoped.hpp>

UIToolbarIcon::UIToolbarIcon(float xPos, float yPos, const std::string& tex, const std::string& texd, std::function<void()> callback)
  : UIFrame(xPos, yPos, 35.0f, 35.0f)
  , texture (tex)
  , textureSelected (texd)
  , _callback (std::move (callback))
  , selected(false)
{
}

UIFrame* UIToolbarIcon::processLeftClick(float /*mx*/, float /*my*/)
{
  _callback();

  return this;
}

void UIToolbarIcon::render() const
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

  if (selected)
  {
    static const float sizer = 18.0f;

    opengl::texture::enable_texture();

    textureSelected->bind();

    gl.begin(GL_TRIANGLE_STRIP);
    gl.texCoord2f(0.0f, 0.0f);
    gl.vertex2f(-sizer, -sizer);
    gl.texCoord2f(1.0f, 0.0f);
    gl.vertex2f(width() + sizer, -sizer);
    gl.texCoord2f(0.0f, 1.0f);
    gl.vertex2f(-sizer, height() + sizer);
    gl.texCoord2f(1.0f, 1.0f);
    gl.vertex2f(width() + sizer, height() + sizer);
    gl.end();

    opengl::texture::disable_texture();
  }
}
