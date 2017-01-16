// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>

class UIAlphamap : public UICloseWindow
{
public:
	UIAlphamap(float x, float y);

	void render() const;

private:
	void drawQuad(size_t i, size_t j) const;

};
