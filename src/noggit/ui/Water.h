// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/tile_index.hpp>
#include <noggit/ui/checkbox.hpp>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QSpinBox;

namespace noggit
{
  namespace ui
  {
    class water : public QWidget
    {
    public:
      water (World*);

      void updatePos(tile_index const& newTile);
      void updateData();

      void changeWaterType(int waterint);

      void paintLiquid (World*, math::vector_3d const& pos, bool add);

      void changeRadius(float change);
      void changeOrientation(float change);
      void changeAngle(float change);
      void change_height(float change) { _lock_pos.y += change; }

      void lockPos(math::vector_3d const& cursor_pos);
      void toggle_lock();
      void toggle_angled_mode();

      float brushRadius() const { return _radius; }
      float angle() const { return _angle; }
      float orientation() const { return _orientation; }
      bool angled_mode() const { return _angled_mode.get(); }
      bool use_ref_pos() const { return _locked.get(); }
      math::vector_3d ref_pos() const { return _lock_pos; }

    private:
      float get_opacity_factor() const;

      int _liquid_id;
      float _radius;

      float _angle;
      float _orientation;

      bool_toggle_property _locked;
      bool_toggle_property _angled_mode;

      bool_toggle_property _override_liquid_id;
      bool_toggle_property _override_height;

      int _opacity_mode;
      float _custom_opacity_factor;

      math::vector_3d _lock_pos;

      QDoubleSpinBox* _radius_spin;
      QDoubleSpinBox* _angle_spin;
      QDoubleSpinBox* _orientation_spin;

      checkbox* _angle_checkbox;
      checkbox* _lock_checkbox;

      QPushButton *waterType;
      QPushButton *cropWater;
      QSpinBox *waterLayer;

      tile_index tile;
    };
  }
}
