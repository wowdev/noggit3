// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texturing_tool.hpp>

#include <noggit/tool_enums.hpp>
#include <noggit/ui/texture_swapper.hpp>
#include <noggit/Misc.h>
#include <noggit/World.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>

namespace ui
{
  texturing_tool::texturing_tool(const math::vector_3d* camera_pos)
    : QWidget(nullptr)
    , _brush_level(255.0f)
    , _hardness(0.5f)
    , _pressure(0.9f)
    , _show_unpaintable_chunks(true)
    , _spray_size(1.0f)
    , _spray_pressure(2.0f)
    , _camera_pos(camera_pos)
  {
    setWindowTitle("Texturing");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto layout (new QFormLayout (this));

    _texture_brush.init();
    _inner_brush.init();
    _spray_brush.init();

    _hardness_spin = new QDoubleSpinBox (this);
    _hardness_spin->setRange (0.0f, 1.0f);
    _hardness_spin->setDecimals (2);
    _hardness_spin->setValue (_hardness);
    _hardness_spin->setSingleStep(0.05f);
    layout->addRow ("Hardness:", _hardness_spin);

    _hardness_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _hardness_slider->setRange (0, 100);
    _hardness_slider->setSliderPosition (_hardness * 100);
    layout->addRow (_hardness_slider);

    _radius_spin = new QDoubleSpinBox (this);
    _radius_spin->setRange (0.0f, 100.0f);
    _radius_spin->setDecimals (2);
    _radius_spin->setValue (_texture_brush.getRadius());
    layout->addRow ("Radius:", _radius_spin);

    _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _radius_slider->setRange (0, 100);
    _radius_slider->setSliderPosition (_texture_brush.getRadius());
    layout->addRow (_radius_slider);

    _pressure_spin = new QDoubleSpinBox (this);
    _pressure_spin->setRange (0.0f, 1.0);
    _pressure_spin->setDecimals (2);
    _pressure_spin->setValue (_pressure);
    _pressure_spin->setSingleStep(0.05f);
    layout->addRow ("Pressure:", _pressure_spin);

    _pressure_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _pressure_slider->setRange (0, 100);
    _pressure_slider->setSliderPosition (std::round(_pressure * 100));
    layout->addRow (_pressure_slider);

    _brush_level_spin = new QDoubleSpinBox (this);
    _brush_level_spin->setRange (0.0f, 255.0f);
    _brush_level_spin->setDecimals (2);
    _brush_level_spin->setValue (_brush_level);
    _brush_level_spin->setSingleStep(5.0f);
    layout->addRow ("Level:", _brush_level_spin);

    _brush_level_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _brush_level_slider->setRange (0, 255);
    _brush_level_slider->setSliderPosition (_brush_level);
    layout->addRow (_brush_level_slider);

    _show_unpaintable_chunks_cb = new QCheckBox("Show unpaintable chunks", this);
    _show_unpaintable_chunks_cb->setChecked(true);
    layout->addRow(_show_unpaintable_chunks_cb);
    
    // spray
    _spray_mode_group = new QGroupBox("Spray", this);
    _spray_mode_group->setCheckable(true);
    layout->addRow (_spray_mode_group);

    _spray_content = new QWidget(_spray_mode_group);
    auto spray_layout (new QFormLayout (_spray_content));
    _spray_mode_group->setLayout(spray_layout);

    _inner_radius_cb = new QCheckBox("Inner radius", _spray_content);
    spray_layout->addRow(_inner_radius_cb);

    _spray_size_spin = new QDoubleSpinBox (_spray_content);
    _spray_size_spin->setRange (1.0f, 40.0f);
    _spray_size_spin->setDecimals (2);
    _spray_size_spin->setValue (_spray_size);
    spray_layout->addRow ("Size:", _spray_size_spin);

    _spray_size_slider = new QSlider (Qt::Orientation::Horizontal, _spray_content);
    _spray_size_slider->setRange (100, 40 * 100);
    _spray_size_slider->setSliderPosition (_spray_size * 100);
    spray_layout->addRow (_spray_size_slider);

    _spray_pressure_spin = new QDoubleSpinBox (_spray_content);
    _spray_pressure_spin->setRange (0.0f, 10.0);
    _spray_pressure_spin->setDecimals (2);
    _spray_pressure_spin->setValue (_spray_pressure);
    spray_layout->addRow ("Pressure:", _spray_pressure_spin);

    _spray_pressure_slider = new QSlider (Qt::Orientation::Horizontal, _spray_content);
    _spray_pressure_slider->setRange (0, 10 * 100);
    _spray_pressure_slider->setSliderPosition (std::round(_spray_pressure * 100));
    spray_layout->addRow (_spray_pressure_slider);

    QPushButton* toggle_swapper = new QPushButton ("Texture swapper", this);
    layout->addRow (toggle_swapper);

    _texture_switcher = new ui::texture_swapper(this, camera_pos);

    connect(toggle_swapper, &QPushButton::clicked, [this]() {
      hide();
      _texture_switcher->show();
    });

    connect ( _radius_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker (_radius_slider);
                set_radius(v);
                _radius_slider->setSliderPosition ((int)std::round (v));
      
              }
            );

    connect ( _radius_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                QSignalBlocker const blocker (_radius_spin);
                set_radius(v);
                _radius_spin->setValue(v);
              }
            );

    connect ( _hardness_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker (_hardness_slider);
                _hardness = v;
                _hardness_slider->setSliderPosition ((int)std::round (v * 100.0f));
                update_brush_hardness();
              }
            );

    connect ( _hardness_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                QSignalBlocker const blocker (_hardness_spin);
                _hardness = v * 0.01f;
                _hardness_spin->setValue(_hardness);
                update_brush_hardness();
              }
            );

    connect ( _pressure_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker (_pressure_slider);
                _pressure = v;
                _pressure_slider->setSliderPosition ((int)std::round (v * 100.0f));
              }
            );

    connect ( _pressure_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                QSignalBlocker const blocker (_pressure_spin);
                _pressure = v * 0.01f;
                _pressure_spin->setValue(_pressure);
              }
            );

    connect ( _brush_level_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker (_brush_level_slider);
                _brush_level = v;
                _brush_level_slider->setSliderPosition ((int)std::round (v));
              }
            );

    connect ( _brush_level_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                QSignalBlocker const blocker (_brush_level_spin);
                _brush_level = v;
                _brush_level_spin->setValue(v);
              }
            );

    connect ( _show_unpaintable_chunks_cb, static_cast<void (QCheckBox::*) (int)> (&QCheckBox::stateChanged)
            , [&] (int state) 
              {
                _show_unpaintable_chunks = state;
              }
            );

    connect ( _spray_size_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker (_spray_size_slider);
                _spray_size = v;
                _spray_size_slider->setSliderPosition ((int)std::round (v * 100.0f));
                update_spray_brush();
              }
            );

    connect ( _spray_size_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                QSignalBlocker const blocker (_spray_size_spin);
                _spray_size = v * 0.01f;
                _spray_size_spin->setValue (_spray_size);      
                update_spray_brush();
              }
            );

    connect ( _spray_pressure_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                QSignalBlocker const blocker (_spray_pressure_slider);
                _spray_pressure = v;
                _spray_pressure_slider->setSliderPosition ((int)std::round (v * 100.0f));
              }
            );

    connect ( _spray_pressure_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                QSignalBlocker const blocker (_spray_pressure_spin);
                _spray_pressure = v * 0.01f;
                _spray_pressure_spin->setValue(_spray_pressure);
              }
            );

    connect ( _spray_mode_group, static_cast<void (QGroupBox::*) (bool)> (&QGroupBox::toggled)
            , [&] (bool b)
              {
                _spray_content->setVisible(b);
                adjustSize();
              }
            );

    _spray_content->hide();
    update_brush_hardness();
    update_spray_brush();
    set_radius(15.0f);
    toggle_spray(); // to disable
  }
  
  void texturing_tool::update_brush_hardness()
  {
    _texture_brush.setHardness(_hardness);
    _inner_brush.setHardness(_hardness);
    _spray_brush.setHardness(_hardness);
  }

  void texturing_tool::set_radius(float radius)
  {
    _texture_brush.setRadius(radius);
    _inner_brush.setRadius(radius * _hardness);
  }

  void texturing_tool::update_spray_brush()
  {
    _spray_brush.setRadius(_spray_size * TEXDETAILSIZE / 2.0f);
  }

  void texturing_tool::toggle_spray()
  {
    _spray_mode_group->setChecked(!_spray_mode_group->isChecked());
  }

  void texturing_tool::change_radius(float change)
  {
    _radius_spin->setValue(_texture_brush.getRadius() + change);
  }

  void texturing_tool::change_hardness(float change)
  {
    _hardness_spin->setValue(_hardness + change);
  }

  void texturing_tool::change_pressure(float change)
  {
    _pressure_spin->setValue(_pressure + change);
  }

  void texturing_tool::change_brush_level(float change)
  {
    _brush_level_spin->setValue(_brush_level + change);
  }

  void texturing_tool::set_brush_level (float level)
  {
    _brush_level_spin->setValue(level);
  }

  void texturing_tool::change_spray_size(float change)
  {
    _spray_size_spin->setValue(_spray_size + change);
  }

  void texturing_tool::change_spray_pressure(float change)
  {
    _spray_pressure_spin->setValue(_spray_pressure + change);
  }

  void texturing_tool::paint(math::vector_3d const& pos, float dt, scoped_blp_texture_reference texture)
  {
    float strength = 1.0f - pow(1.0f - _pressure, dt * 10.0f);

    if (!isVisible() && _texture_switcher->isVisible())
    {
      auto to_swap (_texture_switcher->current_texture());
      if (to_swap)
      {
        gWorld->overwriteTextureAtCurrentChunk(pos, to_swap.get(), texture);
      }      
    }
    else
    {
      if (_spray_mode_group->isChecked())
      {
        gWorld->sprayTexture(pos, &_spray_brush, _brush_level, strength, _texture_brush.getRadius(), _spray_pressure, texture);

        if (_inner_radius_cb->isChecked())
        {
          gWorld->paintTexture(pos, &_inner_brush, _brush_level, strength, texture);
        }
      }
      else
      {
        gWorld->paintTexture(pos, &_texture_brush, _brush_level, strength, texture);
      }
    }
  }

  void texturing_tool::update_brushes()
  {
    if (_texture_brush.needUpdate())
    {
      _texture_brush.GenerateTexture();
      _inner_brush.GenerateTexture();
    }
  }

  void texturing_tool::bind_brush_texture()
  {
    _texture_brush.getTexture()->bind();
  }
}
