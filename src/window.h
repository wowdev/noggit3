#ifndef __WINDOW_H
#define __WINDOW_H

#include "frame.h"
#include "FreeType.h" // fonts.

namespace OpenGL { class Texture; };

class window : public frame
{
protected:
	OpenGL::Texture* texture;

public:
	window( float xPos, float yPos, float w, float h );
	window( float xPos, float yPos, float w, float h, const std::string& pTexture );
	frame *processLeftClick( float mx, float my );
	void render() const;
};

#endif
