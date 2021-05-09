// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/bool_toggle_property.hpp>
#include <noggit/Brush.h>
#include <noggit/TextureManager.h>
#include <noggit/unsigned_int_property.hpp>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

class World;
namespace noggit
{
  namespace ui
  {
    class checkbox;
    class current_texture;
    class texture_swapper;

    enum class texturing_mode
    {
      paint,
      swap,
      anim
    };

    class texturing_tool : public QWidget
    {
    public:
      texturing_tool ( const math::vector_3d* camera_pos
                     , World* world
                     , bool_toggle_property* show_quick_palette
                     , QWidget* parent = nullptr
                     );

      float brush_radius() const;
      float hardness() const;
      bool show_unpaintable_chunks() const;

      void set_brush_level (float level);

      void toggle_tool();

      void change_radius (float change);
      void change_hardness (float change);
      void change_pressure (float change);
      void change_brush_level (float change);
      void set_pressure (float pressure);
	    void toggle_brush_level_min_max();
      void change_spray_size (float change);
      void change_spray_pressure (float change);

      void paint (World* world, math::vector_3d const& pos, float dt, scoped_blp_texture_reference texture);      

      Brush const& texture_brush() const
      {
        return _texture_brush;
      }

      float alpha_target() const
      {
        return static_cast<float>(_brush_level);
      }

      current_texture* _current_texture;

      texture_swapper* const texture_swap_tool() { return _texture_switcher; }

      QSize sizeHint() const override;

    private:
      void change_tex_flag(World* world, math::vector_3d const& pos, bool add, scoped_blp_texture_reference texture);

      // slider functions
      void update_brush_hardness();
      void set_radius (float radius);
      void update_spray_brush();

      Brush _texture_brush;
      Brush _inner_brush;
      Brush _spray_brush;

      int _brush_level;
      float _hardness;
      float _pressure;

      bool _show_unpaintable_chunks;

      float _spray_size;
      float _spray_pressure;

      bool_toggle_property _anim_prop;
      unsigned_int_property _anim_speed_prop;
      unsigned_int_property _anim_rotation_prop;
      bool_toggle_property _overbright_prop;

      texturing_mode _texturing_mode;

    private:
      QSlider* _brush_level_slider;
      QSlider* _hardness_slider;
      QSlider* _radius_slider;
      QSlider* _pressure_slider;
      QSpinBox* _brush_level_spin;
      QDoubleSpinBox* _hardness_spin;
      QDoubleSpinBox* _radius_spin;
      QDoubleSpinBox* _pressure_spin;

      QCheckBox* _show_unpaintable_chunks_cb;

      QGroupBox* _spray_mode_group;
      QWidget* _spray_content;
      QCheckBox* _inner_radius_cb;
      QSlider* _spray_size_slider;
      QSlider* _spray_pressure_slider;
      QDoubleSpinBox* _spray_size_spin;
      QDoubleSpinBox* _spray_pressure_spin;

      QGroupBox* _anim_group;

      texture_swapper* _texture_switcher;
    };
  }
}
