// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texture_swapper.hpp>

#include <math/vector_3d.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/World.h>
#include <noggit/tool_enums.hpp>

#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace noggit
{
  namespace ui
  {
    texture_swapper::texture_swapper ( QWidget* parent
                                     , const math::vector_3d* camera_pos
                                     , World* world
                                     )
      : QWidget (parent)
      , _texture_to_swap()
      , _radius(15.f)
    {
      setWindowTitle ("Swap");
      setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);

      auto layout (new QFormLayout (this));

      _texture_to_swap_display = new current_texture(true, this);

      QPushButton* select = new QPushButton("Select", this);
      QPushButton* swap_adt = new QPushButton("Swap ADT", this);

      layout->addRow(new QLabel("Texture to swap"));
      layout->addRow(_texture_to_swap_display);
      layout->addRow(select);
      layout->addRow(swap_adt);

      _brush_mode_group = new QGroupBox("Brush mode", this);
      _brush_mode_group->setCheckable(true);
      _brush_mode_group->setChecked(false);
      layout->addRow(_brush_mode_group);

      auto brush_content (new QWidget(_brush_mode_group));
      auto brush_layout (new QFormLayout(brush_content));
      _brush_mode_group->setLayout(brush_layout);

      _radius_spin = new QDoubleSpinBox(brush_content);
      _radius_spin->setRange (0.f, 100.f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);
      brush_layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, brush_content);
      _radius_slider->setRange (0, 100);
      _radius_slider->setSliderPosition (_radius);
      brush_layout->addRow (_radius_slider);      
      
      connect(select, &QPushButton::clicked, [&]() {
        _texture_to_swap = selected_texture::get();
        if (_texture_to_swap)
        {
          _texture_to_swap_display->set_texture(_texture_to_swap.get()->filename);
        }
      });

      connect(swap_adt, &QPushButton::clicked, [this, camera_pos, world]() {
        if (_texture_to_swap)
        {
          world->swapTexture (*camera_pos, _texture_to_swap.get());
        }
      });

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&](double v)
                {
                  QSignalBlocker const blocker (_radius_slider);
                  _radius = v;
                  _radius_slider->setSliderPosition ((int)std::round (v));

                }
              );

      connect ( _radius_slider, &QSlider::valueChanged
              , [&](int v)
                {
                  QSignalBlocker const blocker (_radius_spin);
                  _radius = v;
                  _radius_spin->setValue(v);
                }
              );
    }

    void texture_swapper::change_radius(float change)
    {
      _radius_spin->setValue(_radius + change);
    }

    void texture_swapper::set_texture(std::string const& filename)
    {
      _texture_to_swap = std::move(scoped_blp_texture_reference(filename));
    }
  }
}
