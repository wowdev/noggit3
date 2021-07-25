// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/uid_fix_window.hpp>

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace noggit
{
  namespace ui
  {
    uid_fix_window::uid_fix_window()
      : QDialog (nullptr)
    {
      setWindowIcon (QIcon (":/icon"));
      auto layout (new QFormLayout (this));


      auto label1 = new QLabel ("<big><p>In order to avoid issues with duplicating/missing "
        "models and possibly with model culling/collision "
        "unique ID of all objects on a map, or to fix that and "
        "also fix all existing models on the map.</p>"
        "<b>This is only required once.</b></big><br><br>"
        "<h2>For unedited blizzard maps</h2>"
        "<p>This is the fastest method but it will only prevent "
        "new issues from occuring and not fix the current ones.</p>"
        , this
      );
      label1->setWordWrap(true);
      layout->addWidget(label1);
      
      auto get_max (new QPushButton("Get max UID", this));
      layout->addWidget(get_max);

      auto label2 = new QLabel("<hr><h2>Recommended for edited/custom maps</h2>"
        "<p>Takes more time than the max uid method but "
        "it will fix any model duplication, collision and culling issue "
        "while making sure no models have the same id. "
        "It will fail if any model is missing or could not be loaded "
        "to avoid any collision and culling issue.</p>"
        , this
      );
      label2->setWordWrap(true);
      layout->addWidget(label2);

      auto fix_all (new QPushButton("Fix all UIDs", this));
      layout->addWidget(fix_all);

      auto label3 = new QLabel("<hr><font color=red><h2>/!\\ NOT RECOMMENDED</h2>"
        "<h3>USE AT YOUR OWN RISKS</h3></font>"
        "Same as fix all but it will ignore any model loading error "
        "resulting in collision and culling issue. "
        "Any <b>missing or fuckported</b> model will lead to collision and culling issues. "
        "Don't come complaining if you encounter any of those issues afterward."
        , this
      );
      label3->setWordWrap(true);
      layout->addWidget(label3);

      auto fix_all_fuckporting_edition (new QPushButton("Fix all UIDs but potentially fuck up collisions and culling for models", this));
      layout->addWidget(fix_all_fuckporting_edition);      

      connect ( get_max, &QPushButton::clicked
              , [=]
                {
                  hide();
                  emit fix_uid(uid_fix_mode::max_uid);
                  deleteLater();
                }
              );

      connect ( fix_all, &QPushButton::clicked
              , [=]
                {
                  hide();
                  emit fix_uid(uid_fix_mode::fix_all_fail_on_model_loading_error);
                  deleteLater();
                }
              );

      connect ( fix_all_fuckporting_edition, &QPushButton::clicked
              , [=]
                {
                  hide();
                  emit fix_uid(uid_fix_mode::fix_all_fuckporting_edition);
                  deleteLater();
                }
              );
    }
  }
}
