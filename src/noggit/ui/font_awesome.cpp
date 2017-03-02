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

      font_awesome_icon_engine* clone() const
      {
          return new font_awesome_icon_engine (_text);
      }

      virtual void paint (QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
      {
        painter->save();
        {
          QColor color (100, 100, 100);

          if (state == QIcon::On || mode == QIcon::Active)
            color = QColor (0, 0, 0);

          painter->setPen (color);
          painter->setFont (font_awesome::font(rect.height()));
          painter->drawText (rect, _text, QTextOption (Qt::AlignCenter | Qt::AlignVCenter));
        }
        painter->restore ();
      }

      virtual QPixmap pixmap (const QSize &size, QIcon::Mode mode, QIcon::State state)
      {
        QPixmap pm (size);
        pm.fill(Qt::transparent);
        {
            QPainter p (&pm);
            paint (&p, QRect(QPoint(0, 0), size), mode, state);
        }
        return pm;
      }

    private:
      const QString _text;
    };

    QString font_awesome::_family_name = "";

    void font_awesome::load()
    {
      auto id (QFontDatabase::addApplicationFont (":/fonts/FontAwesome.otf"));

      if (id == -1)
      {
        QMessageBox::critical ( nullptr
                              , QObject::tr ("Unable to load FontAwesome")
                              , QObject::tr ("Please make sure to install FontAwesome to your system or place it in the \"fonts\" folder.")
                              );
        
        throw std::runtime_error ("Unable to load FontAwesome");
      }

      _family_name = QFontDatabase::applicationFontFamilies (id).at (0);
    }

    QFont font_awesome::font (int size) {
      QFont font (_family_name);
      font.setPixelSize (size);
      return font;
    }

    font_awesome_icon::font_awesome_icon (const font_awesome::icons& icon)
      : QIcon (new font_awesome_icon_engine (QString (QChar (icon))))
    {}
  }
}