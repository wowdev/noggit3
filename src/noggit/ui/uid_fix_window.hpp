// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QDialog>

enum class uid_fix_mode
{
  none,
  max_uid,
  fix_all_fail_on_model_loading_error,
  fix_all_fuckporting_edition
};

namespace noggit
{
  namespace ui
  {
    class uid_fix_window : public QDialog
    {
    Q_OBJECT

    public:
      uid_fix_window();

    signals:
      void fix_uid (uid_fix_mode uid_fix);
    };
  }
}
