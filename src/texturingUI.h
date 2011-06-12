#ifndef __TEXTURINGUI_H
#define __TEXTURINGUI_H

#include "FreeType.h" // fonts.

class MapChunk;
class frame;
class Gui;
namespace OpenGL { class Texture; };

class TexturingUI
{

public:
  static frame* createSelectedTexture();
  static frame* createTexturePalette(int rows, int cols,Gui* setgui);
  static frame* createTilesetLoader();
  static frame* createTextureFilter();
  static frame* createMapChunkWindow();
  static void setSelectedTexture(OpenGL::Texture* t);
  static OpenGL::Texture* getSelectedTexture();
  static void setChunk(MapChunk *chunk);
  static void setChunkWindow(MapChunk *chunk);
  static void updateSelectedTexture();
  static OpenGL::Texture* selectedTexture;
};

#endif
