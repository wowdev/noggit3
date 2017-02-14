// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/Brush.h>
#include <noggit/ui/Window.h>


class UISlider;
class UICheckBox;
class UIGradient;
class UITextureSwitcher;

namespace ui
{
  class texturing_tool : public UIWindow
  {
  public:
    texturing_tool(float x, float y);

    float brush_radius() { return _texture_brush.getRadius(); }
    float hardness() const { return _hardness; }
    bool highlight_paintable_chunks() const { return _highlight_paintable_chunks; }

    void set_brush_level(float level);

    void toggle_spray();

    void change_radius(float change);
    void change_hardness(float change);
    void change_pressure(float change);
    void change_brush_level(float change);
    void change_spray_size(float change);
    void change_spray_pressure(float change);

    void paint(math::vector_3d const& pos, float dt, scoped_blp_texture_reference texture);

    void update_brushes();
    void bind_brush_texture();

  private:
    // slider functions
    void set_hardness(float hardness);
    void set_radius(float radius);
    void set_spray_size(float size);

    Brush _texture_brush;
    Brush _inner_brush;
    Brush _spray_brush;

    float _brush_level;
    float _hardness;
    float _pressure;

    bool _highlight_paintable_chunks;
    bool _spray_mode;
    bool _paint_inner_radius;

    float _spray_size;
    float _spray_pressure;

  private:
    UIGradient* _brush_level_gradient;
    UISlider* _hardness_slider;
    UISlider* _radius_slider;
    UISlider* _pressure_slider;
    UICheckBox* _highlight_paintable_chunks_cb;
    UICheckBox* _spray_mode_cb;
    UICheckBox* _inner_radius_cb;
    UISlider* _spray_size_slider;
    UISlider* _spray_pressure_slider;
    UITextureSwitcher* _texture_switcher;
  };
}
