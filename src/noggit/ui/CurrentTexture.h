// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>

#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

#include <string>

namespace ui
{
  class current_texture : public QWidget
  {
  private:
    QLabel* _texture;
    std::string _filename;

    virtual void resizeEvent (QResizeEvent * event) { update(); }
    void update();
  public:
    current_texture();
    void set_texture(std::string const& texture);
  };
}
