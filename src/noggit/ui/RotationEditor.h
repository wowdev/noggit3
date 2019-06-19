// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Selection.h>

#include <boost/optional.hpp>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QWidget>
#include <QDockWidget>

namespace noggit
{
  namespace ui
  {
    class rotation_editor : public QWidget
    {
    public:
      rotation_editor(QWidget* parent);

      bool* use_median_pivot_point;

      void select(std::vector<selection_type> entries);
      void updateValues();

    private:
      void update_model(selection_type entry);

	    std::vector<selection_type> _entries;

      QDoubleSpinBox* _rotation_x;
      QDoubleSpinBox* _rotation_z;
      QDoubleSpinBox* _rotation_y;
      QDoubleSpinBox* _position_x;
      QDoubleSpinBox* _position_z;
      QDoubleSpinBox* _position_y;
      QDoubleSpinBox* _scale;
    };
  }
}
