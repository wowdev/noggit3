// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>

#include <noggit/ui/Frame.h>

#include <QtWidgets/QGridLayout>

namespace ui
{
  current_texture::current_texture(UIFrame* texture_palette)
    : QWidget (nullptr)
  {
    setWindowTitle ("Texture");
    setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);

    auto layout (new QGridLayout(this));
    layout->setContentsMargins (QMargins(0, 0, 0, 0));  
    layout->addWidget (_texture = new ui::clickable_label (this), 0, 0);

    _texture->setMinimumSize (64, 64);

    connect (_texture, &ui::clickable_label::clicked, [texture_palette] { texture_palette->toggleVisibility(); });

    set_texture("tileset\\generic\\black.blp");
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
