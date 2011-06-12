#ifndef __ICON_H
#define __ICON_H

#include <string>

#include "frame.h"
#include "UIEventClasses.h"

namespace OpenGL { class Texture; };

class ToolbarIcon : public frame, public UIEventSender
{
public:
  UIEventEventHandlerDefinition(int);
protected:
	OpenGL::Texture* texture;
	OpenGL::Texture* textureSelected;
	int iconId;

public:
	ToolbarIcon( float x, float y, float width, float height, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments );

	void render() const;
	frame *processLeftClick(float mx,float my);
	
	bool selected;
};
#endif
