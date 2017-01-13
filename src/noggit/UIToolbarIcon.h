// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include "UIEventClasses.h"
#include "UIFrame.h"

namespace OpenGL { class Texture; };

class UIToolbarIcon : public UIFrame, public UIEventSender
{
public:
	UIEventEventHandlerDefinition(int);
protected:
	OpenGL::Texture* texture;
	OpenGL::Texture* textureSelected;

	std::string _textureFilename;
	std::string _textureSelectedFilename;

	int iconId;

public:
	UIToolbarIcon(float x, float y, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments);
	~UIToolbarIcon();

	void render() const;
	UIFrame *processLeftClick(float mx, float my);

	bool selected;
};
