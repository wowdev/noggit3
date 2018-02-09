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

      void select(selection_type entry);
      void updateValues();
      bool hasSelection() const { return _selection; }

      bool hasFocus() const {return false;}

    private:
      void update_model();
      math::vector_3d* rotationVect;
      math::vector_3d* posVect;
      float* scale;

      bool _selection;
	  boost::optional<selection_type> _entry;

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
