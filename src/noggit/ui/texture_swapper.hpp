// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/TextureManager.h>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

#include <boost/optional.hpp>

class World;

namespace noggit
{
  namespace ui
  {
    class texture_swapper : public QWidget
    {
    public:
      texture_swapper ( QWidget* parent
                      , const math::vector_3d* camera_pos
                      , World*
                      );

      boost::optional<scoped_blp_texture_reference> const& texture_to_swap() const
      {
        return _texture_to_swap;
      }

      float radius() const
      {
        return _radius;
      }

      void change_radius(float change);

      bool brush_mode() const
      {
        return _brush_mode_group->isChecked();
      }

      void toggle_brush_mode()
      {
        _brush_mode_group->setChecked(!_brush_mode_group->isChecked());
      }

      void set_texture(std::string const& filename);

      current_texture* const texture_display() { return _texture_to_swap_display; }

    private:
      boost::optional<scoped_blp_texture_reference> _texture_to_swap;
      float _radius;

    private:
      current_texture* _texture_to_swap_display;

      QGroupBox* _brush_mode_group;
      QSlider* _radius_slider;
      QDoubleSpinBox* _radius_spin;
    };
  }
}
