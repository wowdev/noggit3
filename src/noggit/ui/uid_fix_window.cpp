// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/uid_fix_window.hpp>

#include <noggit/map_index.hpp>
#include <noggit/World.h>

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    uid_fix_window::uid_fix_window ( std::function<void()> after_fix
                                   , World* world
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
              , [this, after_fix, world]
                {
                  hide();
                  world->mapIndex.fixUIDs (world);
                  after_fix();
                  deleteLater();
                }
              );

      connect ( get_max, &QPushButton::clicked
              , [this, after_fix, world]
                {
                  hide();
                  world->mapIndex.searchMaxUID();
                  after_fix();
                  deleteLater();
                }
              );

      layout()->addWidget (buttons);
    }
  }
}
