#ifndef __TEXTURINGUI_H
#define __TEXTURINGUI_H

class MapChunk;
class UIFrame;
class UIMapViewGUI;

namespace OpenGL { class Texture; };

class UITexturingGUI
{
public:
	static UIFrame* createSelectedTexture();
	static UIFrame* createTexturePalette(UIMapViewGUI* setgui);
	static UIFrame* createTilesetLoader();
	static UIFrame* createTextureFilter();
	static UIFrame* createMapChunkWindow();
	static void setSelectedTexture(OpenGL::Texture* t);
	static OpenGL::Texture* getSelectedTexture();
	static void setChunk(MapChunk *chunk);
	static void setChunkWindow(MapChunk *chunk);
	static void updateSelectedTexture();
	static OpenGL::Texture* selectedTexture;
};

#endif
