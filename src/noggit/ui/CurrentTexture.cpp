// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CurrentTexture.h>

#include <QtWidgets/QGridLayout>

namespace noggit
{
  namespace ui
  {
    current_texture::current_texture()
      : clickable_label (nullptr)
    {
      QSizePolicy policy (QSizePolicy::Preferred, QSizePolicy::Preferred);
      policy.setHeightForWidth (true);
      setSizePolicy (policy);
      setMinimumSize(64, 64);
      set_texture ("tileset\\generic\\black.blp");
    }

    int current_texture::heightForWidth (int width) const
    {
      return width;
    }

    void current_texture::set_texture (std::string const& texture)
    {
      _filename = texture;
      update_texture();
    }

    void current_texture::update_texture()
    {
      show();
      setPixmap (render_blp_to_pixmap (_filename, width(), height()));
    }
  }
}
