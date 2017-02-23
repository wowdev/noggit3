// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Menu.h>
#include <noggit/Video.h>

#include <opengl/context.hpp>

void Menu::display(float /*t*/, float /*dt*/)
{
  video.set2D();
  gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
  gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
