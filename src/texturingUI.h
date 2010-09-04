#ifndef __TEXTURINGUI_H
#define __TEXTURINGUI_H

class MapChunk;

frame *CreateSelectedTexture();
frame *CreateTexturePalette(int rows, int cols,Gui *setgui);
frame *CreateTilesetLoader();
frame *CreateTextureFilter();
frame *createMapChunkWindow();
void setChunk(MapChunk *chunk);
void setChunkWindow(MapChunk *chunk);

#endif
