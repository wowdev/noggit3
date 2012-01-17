// UICloseWindow.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef __CLOSEWINDOWUI_H
#define __CLOSEWINDOWUI_H

#include <string>

#include <noggit/UIWindow.h>

class UICloseWindow : public UIWindow
{
public:
  UICloseWindow( float x, float y, float w, float h, const std::string& pTitle, bool pMoveable = false );
};

#endif
