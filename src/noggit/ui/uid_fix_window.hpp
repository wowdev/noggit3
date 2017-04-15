// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>

#include <QtWidgets/QDialog>

#include <functional>

class World;

enum class uid_fix_mode
{
  none,
  max_uid,
  fix_all
};

namespace noggit
{
  namespace ui
  {
    class uid_fix_window : public QDialog
    {
    Q_OBJECT

    public:
      uid_fix_window (math::vector_3d pos, math::degrees camera_pitch, math::degrees camera_yaw);

    signals:
      void fix_uid  ( math::vector_3d pos
                    , math::degrees camera_pitch
                    , math::degrees camera_yaw
                    , uid_fix_mode uid_fix
                    );
    };
  }
}
