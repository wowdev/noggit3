// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>

#include <QtWidgets/QGridLayout>

namespace ui
{
  current_texture::current_texture()
    : QWidget (nullptr)
  {
    setWindowTitle ("Texture");
    setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);

    auto layout (new QGridLayout(this));
    layout->setContentsMargins (QMargins(0, 0, 0, 0));  
    layout->addWidget (_texture = new QLabel (this), 0, 0);

    _texture->setMinimumSize (64, 64);
  }

  void current_texture::set_texture (std::string const& texture)
  {
    _filename = texture;
    update();
  }

  void current_texture::update()
  {
    show();
    _texture->setPixmap (noggit::render_blp_to_pixmap (_filename, _texture->width(), _texture->height()));
  }
}
