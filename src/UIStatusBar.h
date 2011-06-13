#ifndef __STATUSBAR_H
#define __STATUSBAR_H

#include <string>

#include "UIWindow.h"

class UIText;

class UIStatusBar : public UIWindow
{
private:
  UIText* leftInfo;
  UIText* rightInfo;

public:
  UIStatusBar( float x, float y, float width, float height );
  void render() const;  
  void resize();
  void setLeftInfo( const std::string& pText );
  void setRightInfo( const std::string& pText );
};
#endif
