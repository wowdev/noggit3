// blp_texture.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#pragma once

#include <QString>
#include <QPixmap>

#include <opengl/texture.h>

namespace noggit
{
  namespace detail
  {
    struct blp_header;
  }

  class blp_texture : public opengl::texture
  {
  public:
    blp_texture (const QString& filename);
    blp_texture (std::string const& filename);

    const QString& filename();

  private:
    void from_uncompressed_data (const detail::blp_header*, const char*) const;
    void from_compressed_data (const detail::blp_header*, const char*) const;

    const QString _filename;
  };

  QPixmap render_blp_to_pixmap ( const QString& blp_filename
                               , const int& width
                               , const int& height
                               );
}
