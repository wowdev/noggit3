#ifndef __NOGGIT_BLP_TEXTURE_H
#define __NOGGIT_BLP_TEXTURE_H

#include <QString>
#include <QPixmap>

#include <noggit/Manager.h>

#include <opengl/texture.h>

namespace noggit
{
  namespace detail
  {
    struct blp_header;
  }

  class blp_texture : public ManagedItem, public opengl::texture
  {
  public:
    blp_texture (const QString& filename);

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

#endif
