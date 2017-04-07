// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/TextureManager.h>

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

    private:
      boost::optional<scoped_blp_texture_reference> _texture_to_swap;
    };
  }
}
