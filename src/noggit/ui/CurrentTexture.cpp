// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>

#include <QtWidgets/QGridLayout>

namespace noggit
{
  namespace ui
  {
    current_texture::current_texture()
      : clickable_label (nullptr)
      , _filename("tileset\\generic\\black.blp")
      , _need_update(true)
    {
      QSizePolicy policy (QSizePolicy::Maximum, QSizePolicy::Maximum);
      setSizePolicy (policy);
      setMinimumSize(225, 225);

      update_texture_if_needed();
    }

    int current_texture::heightForWidth (int width) const
    {
      return width;
    }

    void current_texture::set_texture (std::string const& texture)
    {
      _filename = texture;
      _need_update = true;
      update_texture_if_needed();
    }

    void current_texture::update_texture_if_needed()
    {
      if (!_need_update)
      {
        return;
      }

      _need_update = false;

      show();
      setPixmap (render_blp_to_pixmap (_filename, width(), height()));
      setToolTip(QString::fromStdString(_filename));
    }
  }
}
