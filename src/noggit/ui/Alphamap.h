// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>

class UIAlphamap : public UICloseWindow
{
public:
  UIAlphamap(float x, float y, const math::vector_3d* camera_pos);

  void render() const;

private:
  void drawQuad(size_t i, size_t j) const;

  const math::vector_3d* _camera_pos;
};
