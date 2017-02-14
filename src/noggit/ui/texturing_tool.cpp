// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texturing_tool.hpp>

#include <noggit/application.h>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/Button.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/Slider.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Gradient.h>
#include <noggit/ui/TextureSwitcher.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/World.h>


namespace ui
{
  texturing_tool::texturing_tool(float x, float y)
    : UIWindow(x, y, 180.0f, 280.0f)
    , _brush_level(255.0f)
    , _hardness(0.5f)
    , _pressure(0.9f)
    , _highlight_paintable_chunks(true)
    , _spray_mode(false)
    , _paint_inner_radius(false)
    , _spray_size(1.0f)
    , _spray_pressure(2.0f)
  {

    _texture_brush.init();
    _inner_brush.init();
    _spray_brush.init();
    set_hardness(_hardness);
    set_radius(15.0f);
    set_spray_size(_spray_size);

    addChild(new UIText(78.5f, 2.0f, "3D Paint", app.getArial14(), eJustifyCenter));

    _brush_level_gradient = new UIGradient(width() - 24.0f, 4.0f, 20.0f, 92.0f, false);
    _brush_level_gradient->setMaxColor(1.0f, 1.0f, 1.0f, 1.0f);
    _brush_level_gradient->setMinColor(0.0f, 0.0f, 0.0f, 1.0f);
    _brush_level_gradient->setClickColor(1.0f, 0.0f, 0.0f, 1.0f);
    _brush_level_gradient->setClickFunc([&](float f) { _brush_level = (1.0f - f)*255.0f; });
    _brush_level_gradient->setValue(0.0f);
    addChild(_brush_level_gradient);

    _hardness_slider = new UISlider(6.0f, 33.0f, 145.0f, 1.0f, 0.0f);
    _hardness_slider->setFunc([&](float f) { set_hardness(f); });
    _hardness_slider->setValue(_hardness);
    _hardness_slider->setText("Hardness: ");
    addChild(_hardness_slider);

    _radius_slider = new UISlider(6.0f, 59.0f, 145.0f, 100.0f, 0.0f);
    _radius_slider->setFunc([&](float f) { set_radius(f); });
    _radius_slider->setValue(_texture_brush.getRadius() / 100.0f);
    _radius_slider->setText("Radius: ");
    addChild(_radius_slider);

    _pressure_slider = new UISlider(6.0f, 85.0f, 145.0f, 1.0f, 0.0f);
    _pressure_slider->setFunc([&](float f) { _pressure = f; });
    _pressure_slider->setValue(_pressure);
    _pressure_slider->setText("Pressure: ");
    addChild(_pressure_slider);

    _highlight_paintable_chunks_cb = new UICheckBox( 3.0f, 105.0f
                                                   , "Highlight paintable chunks"
                                                   , &_highlight_paintable_chunks
                                                   );
    addChild(_highlight_paintable_chunks_cb);

    _spray_mode_cb = new UICheckBox(3.0f, 138.0f, "Spray", &_spray_mode);
    addChild(_spray_mode_cb);

    _inner_radius_cb = new UICheckBox(80.0f, 138.0f, "Inner radius", &_paint_inner_radius);
    addChild(_inner_radius_cb);

    _spray_size_slider = new UISlider(6.0f, 180.0f, 170.0f, 39.0f, 1.0f);
    _spray_size_slider->setFunc([&](float f) { set_spray_size(f); });
    _spray_size_slider->setValue((_spray_size-1.0f) / 39.0f);
    _spray_size_slider->setText("Spray size: ");
    addChild(_spray_size_slider);

    _spray_pressure_slider = new UISlider(6.0f, 205.0f, 170.0f, 5.0f, 0.0f);
    _spray_pressure_slider->setFunc([&](float f) { _spray_pressure = f; });
    _spray_pressure_slider->setValue(_spray_pressure / 5.0f);
    _spray_pressure_slider->setText("Spray pressure: ");
    addChild(_spray_pressure_slider);

    _texture_switcher = new UITextureSwitcher(x, 40.0f, this);

    addChild(new UIButton( 6.0f , 230.0f, 170.0f, 30.0f
                         , "Texture swapper"
                         , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                         , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                         , [&]
                         {
                           _texture_switcher->show();
                           hide();
                         }));

    addChild(new UIButton( 6.0f, 255.0f, 170.0f, 30.0f
                         , "Remove texture duplicates"
                         , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                         , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                         , [] { gWorld->removeTexDuplicateOnADT(gWorld->camera); }
                         ));
  }
  
  void texturing_tool::set_hardness(float hardness)
  {
    _hardness = std::max(0.0f, std::min(1.0f, hardness));
    _texture_brush.setHardness(_hardness);
    _inner_brush.setHardness(_hardness);
    _spray_brush.setHardness(_hardness);
  }

  void texturing_tool::set_radius(float radius)
  {
    radius = std::max(0.0f, std::min(100.0f, radius));
    _texture_brush.setRadius(radius);
    _inner_brush.setRadius(radius * _hardness);
  }

  void texturing_tool::set_spray_size(float size)
  {
    _spray_size = std::max(1.0f, std::min(40.0f, size));
    _spray_brush.setRadius(_spray_size * TEXDETAILSIZE / 2.0f);
  }

  void texturing_tool::set_brush_level(float level)
  {
    _brush_level = std::max(0.0f, std::min(255.0f, level));
    _brush_level_gradient->setValue(1.0f - (_brush_level / 255.0f));
  }

  void texturing_tool::toggle_spray()
  {
    _spray_mode = !_spray_mode;
    _spray_mode_cb->setState(_spray_mode);
  }

  void texturing_tool::change_radius(float change)
  {
    _radius_slider->setValue((_texture_brush.getRadius() + change) / 100.0f);
  }

  void texturing_tool::change_hardness(float change)
  {
    _hardness_slider->setValue(_hardness + change);
  }

  void texturing_tool::change_pressure(float change)
  {
    _pressure_slider->setValue(_pressure + change);
  }

  void texturing_tool::change_brush_level(float change)
  {
    set_brush_level(_brush_level + change);
  }

  void texturing_tool::change_spray_size(float change)
  {
    _spray_size_slider->setValue((_spray_size + change - 1.0f) / 39.0f);
  }

  void texturing_tool::change_spray_pressure(float change)
  {
    _spray_pressure_slider->setValue((_spray_pressure + change) * 0.2f);
  }

  void texturing_tool::paint(math::vector_3d const& pos, float dt, scoped_blp_texture_reference texture)
  {
    float strength = 1.0f - pow(1.0f - _pressure, dt * 10.0f);

    if (hidden())
    {
      gWorld->overwriteTextureAtCurrentChunk(pos, _texture_switcher->current_texture(), texture);
    }
    else
    {
      if (_spray_mode)
      {
        gWorld->sprayTexture(pos, &_spray_brush, _brush_level, strength, _texture_brush.getRadius(), _spray_pressure, texture);

        if (_paint_inner_radius)
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
