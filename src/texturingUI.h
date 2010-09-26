#ifndef __TEXTURINGUI_H
#define __TEXTURINGUI_H

class MapChunk;
class frame;
class Gui;
class Texture;

class TexturingUI
{
public:
  static frame* createSelectedTexture();
  static frame* createTexturePalette(int rows, int cols,Gui* setgui);
  static frame* createTilesetLoader();
  static frame* createTextureFilter();
  static frame* createMapChunkWindow();
  static void setChunk(MapChunk *chunk);
  static void setChunkWindow(MapChunk *chunk);

};

static Texture* selectedTexture;

#endif
