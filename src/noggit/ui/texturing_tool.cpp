// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texturing_tool.hpp>

#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/checkbox.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/texture_swapper.hpp>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>

namespace noggit
{
  namespace ui
  {
    texturing_tool::texturing_tool ( const math::vector_3d* camera_pos
                                   , World* world
                                   )
      : QWidget(nullptr)
      , _brush_level(255.0f)
      , _hardness(0.5f)
      , _pressure(0.9f)
      , _show_unpaintable_chunks(true)
      , _spray_size(1.0f)
      , _spray_pressure(2.0f)
      , _texturing_mode(texturing_mode::paint)
      , _anim_prop(true)
      , _anim_speed_prop(1)
      , _anim_rotation_prop(4)
      , _overbright_prop(false)
    {
      auto layout (new QFormLayout (this));

      _texture_brush.init();
      _inner_brush.init();
      _spray_brush.init();

      _current_texture = new current_texture;
      layout->addRow (_current_texture);

      auto tabs (new QTabWidget(this));

      auto tool_widget (new QWidget (this));
      auto tool_layout (new QFormLayout (tool_widget));

      _hardness_spin = new QDoubleSpinBox (tool_widget);
      _hardness_spin->setRange (0.0f, 1.0f);
      _hardness_spin->setDecimals (2);
      _hardness_spin->setValue (_hardness);
      _hardness_spin->setSingleStep(0.05f);
      tool_layout->addRow ("Hardness:", _hardness_spin);

      _hardness_slider = new QSlider (Qt::Orientation::Horizontal, tool_widget);
      _hardness_slider->setRange (0, 100);
      _hardness_slider->setSliderPosition (_hardness * 100);
      tool_layout->addRow (_hardness_slider);

      _radius_spin = new QDoubleSpinBox (tool_widget);
      _radius_spin->setRange (0.0f, 100.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_texture_brush.getRadius());
      tool_layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, tool_widget);
      _radius_slider->setRange (0, 100);
      _radius_slider->setSliderPosition (_texture_brush.getRadius());
      tool_layout->addRow (_radius_slider);

      _pressure_spin = new QDoubleSpinBox (tool_widget);
      _pressure_spin->setRange (0.0f, 1.0);
      _pressure_spin->setDecimals (2);
      _pressure_spin->setValue (_pressure);
      _pressure_spin->setSingleStep(0.05f);
      tool_layout->addRow ("Pressure:", _pressure_spin);

      _pressure_slider = new QSlider (Qt::Orientation::Horizontal, tool_widget);
      _pressure_slider->setRange (0, 100);
      _pressure_slider->setSliderPosition (std::round(_pressure * 100));
      tool_layout->addRow (_pressure_slider);

      _brush_level_spin = new QDoubleSpinBox (tool_widget);
      _brush_level_spin->setRange (0.0f, 255.0f);
      _brush_level_spin->setDecimals (2);
      _brush_level_spin->setValue (_brush_level);
      _brush_level_spin->setSingleStep(5.0f);
      tool_layout->addRow ("Level:", _brush_level_spin);

      _brush_level_slider = new QSlider (Qt::Orientation::Horizontal, tool_widget);
      _brush_level_slider->setRange (0, 255);
      _brush_level_slider->setSliderPosition (_brush_level);
      tool_layout->addRow (_brush_level_slider);

      _show_unpaintable_chunks_cb = new QCheckBox("Show unpaintable chunks", tool_widget);
      _show_unpaintable_chunks_cb->setChecked(true);
      tool_layout->addRow(_show_unpaintable_chunks_cb);

      // spray
      _spray_mode_group = new QGroupBox("Spray", tool_widget);
      _spray_mode_group->setCheckable(true);
      tool_layout->addRow (_spray_mode_group);

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

      _texture_switcher = new texture_swapper(tool_widget, camera_pos, world);
      _texture_switcher->hide();


      auto anim_widget (new QWidget (this));
      auto anim_layout (new QFormLayout (anim_widget));

      auto anim_group (new QGroupBox("Add anim", anim_widget));
      anim_group->setCheckable(true);
      anim_group->setChecked(_anim_prop.get());

      auto anim_group_layout (new QFormLayout (anim_group));

      auto anim_speed_slider = new QSlider(Qt::Orientation::Horizontal, anim_group);
      anim_speed_slider->setRange(0, 7);
      anim_speed_slider->setSingleStep(1);
      anim_speed_slider->setTickInterval(1);
      anim_speed_slider->setTickPosition(QSlider::TickPosition::TicksBothSides);
      anim_speed_slider->setValue(_anim_speed_prop.get());
      anim_group_layout->addRow("Speed:", anim_speed_slider);

      anim_group_layout->addRow(new QLabel("Orientation:", anim_group));

      auto anim_orientation_dial = new QDial(anim_group);
      anim_orientation_dial->setRange(0, 8);
      anim_orientation_dial->setSingleStep(1);
      anim_orientation_dial->setValue(_anim_rotation_prop.get());
      anim_orientation_dial->setWrapping(true);
      anim_group_layout->addRow(anim_orientation_dial);

      anim_layout->addRow(anim_group);

      auto overbright_cb = new checkbox("Overbright", &_overbright_prop, anim_widget);
      anim_layout->addRow(overbright_cb);

      tabs->addTab(tool_widget, "Paint");
      tabs->addTab(_texture_switcher, "Swap");
      tabs->addTab(anim_widget, "Anim");
      
      layout->addRow(tabs);

      connect (anim_group, &QGroupBox::toggled, &_anim_prop, &noggit::bool_toggle_property::set);
      connect (anim_speed_slider, &QSlider::valueChanged, &_anim_speed_prop, &noggit::unsigned_int_property::set);
      connect (anim_orientation_dial, &QDial::valueChanged, &_anim_rotation_prop, &noggit::unsigned_int_property::set);

      connect ( tabs, &QTabWidget::currentChanged
              , [this] (int index)
                {
                  switch (index)
                  {
                    case 0: _texturing_mode = texturing_mode::paint; break;
                    case 1: _texturing_mode = texturing_mode::swap; break;
                    case 2: _texturing_mode = texturing_mode::anim; break;
                  }
                }
              );

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_radius_slider);
                  set_radius(v);
                  _radius_slider->setSliderPosition ((int)std::round (v));

                }
              );

      connect ( _radius_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_radius_spin);
                  set_radius(v);
                  _radius_spin->setValue(v);
                }
              );

      connect ( _hardness_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_hardness_slider);
                  _hardness = v;
                  _hardness_slider->setSliderPosition ((int)std::round (v * 100.0f));
                  update_brush_hardness();
                }
              );

      connect ( _hardness_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_hardness_spin);
                  _hardness = v * 0.01f;
                  _hardness_spin->setValue(_hardness);
                  update_brush_hardness();
                }
              );

      connect ( _pressure_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_pressure_slider);
                  _pressure = v;
                  _pressure_slider->setSliderPosition ((int)std::round (v * 100.0f));
                }
              );

      connect ( _pressure_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_pressure_spin);
                  _pressure = v * 0.01f;
                  _pressure_spin->setValue(_pressure);
                }
              );

      connect ( _brush_level_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_brush_level_slider);
                  _brush_level = v;
                  _brush_level_slider->setSliderPosition ((int)std::round (v));
                }
              );

      connect ( _brush_level_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_brush_level_spin);
                  _brush_level = v;
                  _brush_level_spin->setValue(v);
                }
              );

      connect ( _show_unpaintable_chunks_cb, &QCheckBox::stateChanged
              , [&] (int state)
                {
                  _show_unpaintable_chunks = state;
                }
              );

      connect ( _spray_size_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_spray_size_slider);
                  _spray_size = v;
                  _spray_size_slider->setSliderPosition ((int)std::round (v * 100.0f));
                  update_spray_brush();
                }
              );

      connect ( _spray_size_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_spray_size_spin);
                  _spray_size = v * 0.01f;
                  _spray_size_spin->setValue (_spray_size);
                  update_spray_brush();
                }
              );

      connect ( _spray_pressure_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  QSignalBlocker const blocker (_spray_pressure_slider);
                  _spray_pressure = v;
                  _spray_pressure_slider->setSliderPosition ((int)std::round (v * 100.0f));
                }
              );

      connect ( _spray_pressure_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  QSignalBlocker const blocker (_spray_pressure_spin);
                  _spray_pressure = v * 0.01f;
                  _spray_pressure_spin->setValue(_spray_pressure);
                }
              );

      connect ( _spray_mode_group, &QGroupBox::toggled
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

    float texturing_tool::brush_radius() const
    {
      // show only a dot when using the anim / swap mode
      return (_texturing_mode == texturing_mode::paint ? _texture_brush.getRadius() : 0.0f);
    }

    bool texturing_tool::show_unpaintable_chunks() const
    { 
      return _show_unpaintable_chunks && _texturing_mode == texturing_mode::paint; 
    }

    void texturing_tool::paint (World* world, math::vector_3d const& pos, float dt, scoped_blp_texture_reference texture)
    {
      float strength = 1.0f - pow(1.0f - _pressure, dt * 10.0f);

      if (_texturing_mode == texturing_mode::swap)
      {
        auto to_swap (_texture_switcher->texture_to_swap());
        if (to_swap)
        {
          world->overwriteTextureAtCurrentChunk(pos, to_swap.get(), texture);
        }
      }
      else if (_texturing_mode == texturing_mode::paint)
      {
        if (_spray_mode_group->isChecked())
        {
          world->sprayTexture(pos, &_spray_brush, _brush_level, strength, _texture_brush.getRadius(), _spray_pressure, texture);

          if (_inner_radius_cb->isChecked())
          {
            world->paintTexture(pos, &_inner_brush, _brush_level, strength, texture);
          }
        }
        else
        {
          world->paintTexture(pos, &_texture_brush, _brush_level, strength, texture);
        }
      }
    }

    void texturing_tool::change_tex_flag(World* world, math::vector_3d const& pos, bool add, scoped_blp_texture_reference texture)
    {
      if (_texturing_mode == texturing_mode::anim)
      {
        std::size_t flag = 0;
        if (_anim_prop.get())
        {
          flag |= FLAG_ANIMATE;
          if (add)
          {
            // the qdial in inverted compared to the anim rotation
            flag |= ( _anim_rotation_prop.get() + 4 ) % 8;
            flag |= _anim_speed_prop.get() << 3;
          }
        }
        if (_overbright_prop.get())
        {
          flag |= FLAG_GLOW;
        }

        world->change_texture_flag(pos, texture, flag, add);
      }
    }
  }
}
