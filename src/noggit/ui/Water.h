// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/bool_toggle_property.hpp>
#include <noggit/tile_index.hpp>
#include <noggit/ui/checkbox.hpp>
#include <noggit/unsigned_int_property.hpp>

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class World;
class QComboBox;

namespace noggit
{
  namespace ui
  {
    class water : public QWidget
    {
      Q_OBJECT

    public:
      water ( unsigned_int_property* current_layer
            , bool_toggle_property* display_all_layers
            , QWidget* parent = nullptr
            );

      void updatePos(tile_index const& newTile);
      void updateData();

      void changeWaterType(int waterint);

      void paintLiquid (World*, math::vector_3d const& pos, bool add);

      void changeRadius(float change);
      void changeOrientation(float change);
      void changeAngle(float change);
      void change_height(float change);

      void lockPos(math::vector_3d const& cursor_pos);
      void toggle_lock();
      void toggle_angled_mode();

      float brushRadius() const { return _radius; }
      float angle() const { return _angle; }
      float orientation() const { return _orientation; }
      bool angled_mode() const { return _angled_mode.get(); }
      bool use_ref_pos() const { return _locked.get(); }
      math::vector_3d ref_pos() const { return _lock_pos; }

      QSize sizeHint() const override;

    signals:
      void regenerate_water_opacity (float factor);
      void crop_water();

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

      QDoubleSpinBox* _x_spin;
      QDoubleSpinBox* _z_spin;
      QDoubleSpinBox* _h_spin;

      QComboBox* waterType;
      QSpinBox* waterLayer;

      tile_index tile;
    };
  }
}
