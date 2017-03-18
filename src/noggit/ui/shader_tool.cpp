// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>
#include <noggit/ui/shader_tool.hpp>
#include <util/qt/overload.hpp>

#include <qt-color-widgets/color_selector.hpp>

#include <QtWidgets/QFormLayout>

namespace noggit
{
  namespace ui
  {
    shader_tool::shader_tool(math::vector_4d& color)
      : QWidget(nullptr)
      , _radius(15.0f)
      , _speed(1.0f)
      , _color(color)
    {
      auto layout (new QFormLayout(this));

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.0f, 100.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);

      layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _radius_slider->setRange (0, 100);
      _radius_slider->setSliderPosition (_radius);

      layout->addRow (_radius_slider);

      _speed_spin = new QDoubleSpinBox (this);
      _speed_spin->setRange (0.0f, 10.0f);
      _speed_spin->setDecimals (2);
      _speed_spin->setValue (_speed);

      layout->addRow ("Speed:", _speed_spin);

      _speed_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _speed_slider->setRange (0, 10 * 100);
      _speed_slider->setSingleStep (50);
      _speed_slider->setSliderPosition (_speed * 100);

      layout->addRow(_speed_slider);

      auto color_picker (new color_widgets::ColorSelector (this));
      color_picker->setDisplayMode (color_widgets::ColorSelector::NoAlpha);
      color_picker->setColor (QColor::fromRgbF (color.x, color.y, color.z, color.w));

      layout->addRow("Color:", color_picker);

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_slider);
                  _radius_slider->setSliderPosition ((int)std::round (v));
                }
              );

      connect ( _radius_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_spin);
                  _radius_spin->setValue(v);
                }
              );

      connect ( _speed_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  _speed = v;
                  QSignalBlocker const blocker(_speed_slider);
                  _speed_slider->setSliderPosition ((int)std::round (v * 100.0f));
                }
              );

      connect ( _speed_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  _speed = v / 100.0f;
                  QSignalBlocker const blocker(_speed_spin);
                  _speed_spin->setValue (_speed);
                }
              );

      connect ( color_picker, &color_widgets::ColorSelector::colorChanged
              , [&] (QColor new_color)
                {
                  _color.x = new_color.redF();
                  _color.y = new_color.greenF();
                  _color.z = new_color.blueF();
                  _color.w = 1.0f;
                }
              );

    }

    void shader_tool::changeShader
      (World* world, math::vector_3d const& pos, float dt, bool add)
    {
      world->changeShader (pos, _color, 2.0f*dt*_speed, _radius, add);
    }

    void shader_tool::changeRadius(float change)
    {
      _radius_spin->setValue (_radius + change);
    }

    void shader_tool::changeSpeed(float change)
    {
      _speed_spin->setValue(_speed + change);
    }
  }
}
