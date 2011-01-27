#ifndef __ICON_H
#define __ICON_H

#include "frame.h"
#include "UIEventClasses.h"

class Texture;

class ToolbarIcon : public frame, public UIEventSender
{
public:
	typedef void (UIEventListener::*EventHandlerType)(int);
protected:
	Texture* texture;
	Texture* textureSelected;
	int iconId;

public:
	ToolbarIcon( float x, float y, float width, float height, const std::string& tex, const std::string& texd, const int& id, EventHandlerType _eventHandler, UIEventListener* _listener );

	void render() const;
	frame *processLeftClick(float mx,float my);
	
	bool selected;
};
#endif
