// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <QtCore/QString>
#include <QtGui/QIconEngine>
#include <QtGui/QFontDatabase>
#include <QtGui/QPainter>
#include <QtWidgets/QMessageBox>

#include <noggit/ui/font_awesome.hpp>
#include <noggit/Log.h>

namespace noggit
{
  namespace ui
  {
    class font_awesome_icon_engine : public QIconEngine
    {
    public:
      font_awesome_icon_engine (const QString& text)
        : _text (text)
      {}

      virtual font_awesome_icon_engine* clone() const override
      {
        return new font_awesome_icon_engine (_text);
      }

      virtual void paint ( QPainter* painter
                         , QRect const& rect
                         , QIcon::Mode mode
                         , QIcon::State state
                         ) override
      {
        painter->save();
        {
          painter->setPen ( (state == QIcon::On || mode == QIcon::Active)
                          ? QColor (0, 0, 0)
                          : QColor (100, 100, 100)
                          );

          if (!_fonts.count (rect.height()))
          {
            auto id (QFontDatabase::addApplicationFont (":/fonts/FontAwesome.otf"));

            if (id == -1)
            {
              throw std::runtime_error ("Unable to load FontAwesome");
            }

            QFont font (QFontDatabase::applicationFontFamilies (id).at (0));
            font.setPixelSize (rect.height());

            _fonts[rect.height()] = font;
          }

          painter->setFont (_fonts.at (rect.height()));

          painter->drawText
            (rect, _text, QTextOption (Qt::AlignCenter | Qt::AlignVCenter));
        }
        painter->restore();
      }

      virtual QPixmap pixmap ( QSize const& size
                             , QIcon::Mode mode
                             , QIcon::State state
                             ) override
      {
        QPixmap pm (size);
        pm.fill (Qt::transparent);
        {
          QPainter p (&pm);
          paint (&p, QRect(QPoint(0, 0), size), mode, state);
        }
        return pm;
      }

    private:
      const QString _text;

      static std::map<int, QFont> _fonts;
    };

    std::map<int, QFont> font_awesome_icon_engine::_fonts = {};

    font_awesome_icon::font_awesome_icon (font_awesome::icons const& icon)
      : QIcon (new font_awesome_icon_engine (QString (QChar (icon))))
    {}
  }
}
