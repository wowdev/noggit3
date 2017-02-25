// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/HelperModels.h>

#include <noggit/TextureManager.h>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

UIHelperModels::UIHelperModels(MapView *mapview)
  : QWidget (nullptr)
{
  auto layout (new QVBoxLayout (this));
  auto top_layout (new QHBoxLayout (nullptr));
  layout->addLayout (top_layout);
  auto bottom_layout (new QGridLayout (nullptr));
  layout->addLayout (bottom_layout);

  auto icon (new QLabel (this));
  icon->setPixmap (noggit::render_blp_to_pixmap ("interface/icons/inv_misc_enggizmos_swissarmy.blp"));
  top_layout->addWidget (icon);
  top_layout->addWidget (new QLabel ("Select a model to add.\nYou should select a chunk first.", this));

  auto add_button
    ( [&] (char const* label, int id, int pos_x, int pos_y)
      {
        auto button (new QPushButton (label, this));

        connect ( button, &QPushButton::clicked
                , [=]
                  {
                    mapview->inserObjectFromExtern (id);
                  }
                );

        bottom_layout->addWidget (button, pos_y, pos_x);
      }
    );

  add_button ("Human scale", 2, 0, 0);
  add_button ("Cube 50", 3, 0, 1);
  add_button ("Cube 100", 4, 0, 2);
  add_button ("Cube 250", 5, 0, 3);
  add_button ("Cube 500", 6, 0, 4);
  add_button ("Cube 1000", 7, 0, 5);

  add_button ("Disc 50", 8, 1, 0);
  add_button ("Disc 200", 9, 1, 1);
  add_button ("Disc 777", 10, 1, 2);
  add_button ("Sphere 50", 11, 1, 3);
  add_button ("Sphere 200", 12, 1, 4);
  add_button ("Sphere 777", 13, 1, 5);
}
