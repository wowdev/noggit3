// UIStatusBar.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef __STATUSBAR_H
#define __STATUSBAR_H

#include <string>

#include <noggit/UIWindow.h>

class UIText;

class UIStatusBar : public UIWindow
{
private:
  UIText* leftInfo;
  UIText* rightInfo;

public:
  UIStatusBar( float x, float y, float width, float height );
  void render() const;
  void setLeftInfo( const std::string& pText );
  void setRightInfo( const std::string& pText );
};
#endif
