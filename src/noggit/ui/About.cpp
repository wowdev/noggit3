// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/About.h>
#include <noggit/TextureManager.h>

#include "revision.h"

#include <QIcon>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    about::about(QWidget* parent)
      : QDialog(parent)
    {
      setWindowIcon(QIcon(":/icon"));
      setWindowTitle("About");

      //! \todo make nice looking again, I don't care currently
      new QVBoxLayout (this);

      auto icon (new QLabel (this));
      icon->setPixmap (render_blp_to_pixmap ("interface/icons/inv_potion_83.blp"));
      layout()->addWidget (icon);
      //! \todo was Skurri32
      layout()->addWidget (new QLabel ("Noggit Studio", this));
      layout()->addWidget (new QLabel ("a wow map editor for 3.3.5a", this));
      layout()->addWidget ( new QLabel ( "Ufoz [...], Cryect, Beket, Schlumpf, "
                                         "Tigurius, Steff, Garthog, Glararan, Cromon, "
                                         "Hanfer, Skarn, AxelSheva, Valium, Kaev, "
                                         "Adspartan", this
                                       )
                          );
      layout()->addWidget (new QLabel ("World of Warcraft is (C) Blizzard Entertainment", this));
      layout()->addWidget (new QLabel (STRPRODUCTVER, this));
      layout()->addWidget (new QLabel (__DATE__ ", " __TIME__, this));
    }
  }
}
