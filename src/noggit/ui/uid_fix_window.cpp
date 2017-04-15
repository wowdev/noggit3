// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/uid_fix_window.hpp>

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    uid_fix_window::uid_fix_window ( math::vector_3d pos
                                   , math::degrees camera_pitch
                                   , math::degrees camera_yaw
                                   )
      : QDialog (nullptr)
    {
      new QVBoxLayout (this);

      layout()->addWidget
        ( new QLabel ( "In order to avoid issues with duplicating or missing\n"
                       "models, it is required to either determine the maximum\n"
                       "unique ID of all objects on a map, or to fix do that and\n"
                       "also fix all existing models on the map.\n"
                       "\n"
                       "For existing maps that have been previously edited it is\n"
                       "recommended to fix up, while for untouched, Blizzard maps\n"
                       "it is sufficient to only determine the currently highest\n"
                       "unique ID. Getting the currently maximum unique ID will\n"
                       "only prevent new issues, while fixing all takes a few\n"
                       "seconds and will also remove existing issues.\n"
                       "\n"
                       "This is only required once."
                     , this
                     )
        );

      auto buttons (new QDialogButtonBox (this));
      auto fix_all (buttons->addButton ("Fix All", QDialogButtonBox::AcceptRole));
      auto get_max (buttons->addButton ("Get Max UID", QDialogButtonBox::YesRole));

      connect ( fix_all, &QPushButton::clicked
              , [=]
                {
                  hide();
                  emit fix_uid(pos, camera_pitch, camera_yaw, uid_fix_mode::fix_all);
                  deleteLater();
                }
              );

      connect ( get_max, &QPushButton::clicked
              , [=]
                {
                  hide();
                  emit fix_uid(pos, camera_pitch, camera_yaw, uid_fix_mode::max_uid);
                  deleteLater();
                }
              );

      layout()->addWidget (buttons);
    }
  }
}
