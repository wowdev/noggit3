// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>
#include <noggit/ui/widget.hpp>

#include <boost/optional.hpp>

class UIFrame;
class UIMapViewGUI;

namespace noggit
{
  namespace ui
  {
    class current_texture;

    struct tileset_chooser : public widget
    {
      Q_OBJECT

    public:
      tileset_chooser (QWidget* parent = nullptr);

    signals:
      void selected (std::string);
    };
  }
}

class UITexturingGUI
{
public:
  static void setSelectedTexture(scoped_blp_texture_reference t);
  static boost::optional<scoped_blp_texture_reference> getSelectedTexture();
  static boost::optional<scoped_blp_texture_reference> selectedTexture;
};
