// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_4d.hpp>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

namespace noggit
{
  namespace ui
  {
    class shader_tool : public QWidget
    {
    public:
      shader_tool(math::vector_4d& color);

      void changeShader (math::vector_3d const& pos, float dt, bool add);

      void changeRadius(float change);
      void changeSpeed(float change);

      float brushRadius() const { return _radius; }
  
    private:
      float _radius;
      float _speed;
      math::vector_4d& _color;

      QSlider* _radius_slider;
      QSlider* _speed_slider;
      QDoubleSpinBox* _radius_spin;
      QDoubleSpinBox* _speed_spin;
    };
  }
}
