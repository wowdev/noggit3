// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>

#include <boost/optional.hpp>

class UIFrame;
class UIMapViewGUI;

namespace noggit
{
  namespace ui
  {
    class current_texture;
  }
}

class UITexturingGUI
{
public:
  static UIFrame* createTexturePalette (noggit::ui::current_texture*);
  static UIFrame* createTilesetLoader();
  static UIFrame* createTextureFilter();
  static void setSelectedTexture(scoped_blp_texture_reference t);
  static boost::optional<scoped_blp_texture_reference> getSelectedTexture();
  static void updateSelectedTexture (noggit::ui::current_texture*);
  static boost::optional<scoped_blp_texture_reference> selectedTexture;
};
