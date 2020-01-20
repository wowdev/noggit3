// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/HelperModels.h>

#include <noggit/MapView.h>
#include <noggit/TextureManager.h>
#include <noggit/ui/ObjectEditor.h>

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    helper_models::helper_models(object_editor* object_editor)
      : QWidget (object_editor, Qt::Tool | Qt::WindowStaysOnTopHint)
    {
      setWindowIcon (QIcon (":/icon"));
      auto layout (new QVBoxLayout (this));
      auto top_layout (new QHBoxLayout (nullptr));
      layout->addLayout (top_layout);
      auto bottom_layout (new QGridLayout (nullptr));
      layout->addLayout (bottom_layout);

      auto icon (new QLabel (this));
      icon->setPixmap (render_blp_to_pixmap ("interface/icons/inv_misc_enggizmos_swissarmy.blp"));
      top_layout->addWidget (icon);
      top_layout->addWidget (new QLabel ("Select a model to add.\nYou should select a chunk first.", this));

      auto add_button
        ( [&] (char const* label, std::string path, int pos_x, int pos_y)
          {
            auto button (new QPushButton (label, this));

            connect ( button, &QPushButton::clicked
                    , [=]
                      {
                        object_editor->copy(path);
                      }
                    );

            bottom_layout->addWidget (button, pos_y, pos_x);
          }
        );

      add_button ("Human scale", "world/scale/humanmalescale.m2", 0, 0);
      add_button ("Cube 50", "world/scale/50x50.m2", 0, 1);
      add_button ("Cube 100", "world/scale/100x100.m2", 0, 2);
      add_button ("Cube 250", "world/scale/250x250.m2", 0, 3);
      add_button ("Cube 500", "world/scale/500x500.m2", 0, 4);
      add_button ("Cube 1000", "world/scale/1000x1000.m2", 0, 5);

      add_button ("Disc 50 radius", "world/scale/50yardradiusdisc.m2", 1, 0);
      add_button ("Disc 200 radius", "world/scale/200yardradiusdisc.m2", 1, 1);
      add_button ("Disc 777 radius", "world/scale/777yardradiusdisc.m2", 1, 2);
      add_button ("Sphere 50 radius", "world/scale/50yardradiussphere.m2", 1, 3);
      add_button ("Sphere 200 radius", "world/scale/200yardradiussphere.m2", 1, 4);
      add_button ("Sphere 777 radius", "world/scale/777yardradiussphere.m2", 1, 5);
    }
  }
}
