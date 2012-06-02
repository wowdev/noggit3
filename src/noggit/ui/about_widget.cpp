// about_widget.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>
// Glararan <glararan@glararan.eu>

#include <noggit/ui/about_widget.h>

#include <QLabel>

#include <helper/repository.h>
#include <noggit/blp_texture.h>

namespace noggit
{
  namespace ui
  {
    about_widget::about_widget (QWidget* parent)
      : QWidget (parent)
    {
      setMinimumSize (500, 260);
      setMaximumSize (500, 260);

      setWindowTitle (tr ("About Noggit"));

      QPalette p (palette());
      p.setColor (QPalette::Background, QColor ("#002020"));
      p.setColor (QPalette::WindowText, Qt::white);
      setPalette (p);

      QLabel* icon (new QLabel (this));
      QLabel* title (new QLabel (tr ("Noggit Studio"), this));
      QLabel* description (new QLabel (tr ("a WoW map editor for 3.3.5a"), this));
      QLabel* authors (new QLabel (tr ("Maintained by %1").arg ("Ufoz, [...], Cryect, Beket, Schlumpf, Tigurius, Steff, Garthog, Glararan, Hanfer & Mjollna"), this));
      QLabel* copyright (new QLabel (trUtf8 ("World of Warcraft © Blizzard Entertainment"), this));
      QLabel* product_version (new QLabel (helper::repository::revision_string(), this));
      QLabel* date (new QLabel (tr ("%1, %2").arg (__DATE__).arg (__TIME__), this));

      static QFont skurri32 ("Skurri", 32);
      static QFont friz16 ("Friz Quadrata TT", 16);

      icon->setPixmap (noggit::render_blp_to_pixmap ("Interface\\ICONS\\INV_Potion_83.blp", 64, 64));
      icon->setGeometry (20, 20, 64, 64);
      icon->setPalette (p);
      title->move (73, 24);
      title->setFont (skurri32);
      title->setPalette (p);
      description->move (160, 70);
      description->setFont (friz16);
      description->setPalette (p);
      authors->move (20, 125);
      authors->setWordWrap (true);
      authors->setFont (friz16);
      authors->setPalette (p);

      copyright->move (20, 205);
      copyright->setPalette (p);
      product_version->move (20, 220);
      product_version->setPalette (p);
      date->setGeometry (10, 220, 400, 16);
      date->setAlignment (Qt::AlignRight | Qt::AlignBottom);
      date->setPalette (p);
    }
  }
}
