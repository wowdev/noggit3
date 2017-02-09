// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/TextureManager.h>

#include <boost/optional.hpp>

class MapChunk;
class UIFrame;
class UIMapViewGUI;

class UITexturingGUI
{
public:
  static UIFrame* createTexturePalette(UIMapViewGUI* setgui);
  static UIFrame* createTilesetLoader();
  static UIFrame* createTextureFilter();
  static UIFrame* createMapChunkWindow();
  static void setSelectedTexture(scoped_blp_texture_reference t);
  static boost::optional<scoped_blp_texture_reference> getSelectedTexture();
  static void setChunk(MapChunk *chunk);
  static void setChunkWindow(MapChunk *chunk);
  static void updateSelectedTexture();
  static boost::optional<scoped_blp_texture_reference> selectedTexture;
};
