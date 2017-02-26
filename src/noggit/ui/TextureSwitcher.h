// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/TextureManager.h>

#include <QtWidgets/QWidget>

#include <boost/optional.hpp>

class UItexture_swapper : public QWidget
{
public:
  UItexture_swapper (QWidget* parent, const math::vector_3d* camera_pos);

  boost::optional<scoped_blp_texture_reference> const& current_texture() const
  {
    return _texture_to_swap;
  }

private:
  void closeEvent(QCloseEvent *event);

  boost::optional<scoped_blp_texture_reference> _texture_to_swap;
};
