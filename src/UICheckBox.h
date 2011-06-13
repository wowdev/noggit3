#ifndef __CHECKBOXUI_H
#define __CHECKBOXUI_H

#include <string>

#include "UIFrame.h"

class UITexture;
class UIText;
class UIToggleGroup;

class UICheckBox : public UIFrame
{
protected:
  UITexture* check;
  UIText* text;
  bool checked;
  int id;
  void (*clickFunc)( bool, int );

  UIToggleGroup* mToggleGroup;
  
public:
  UICheckBox( float, float, const std::string& );
  UICheckBox( float, float, const std::string&, UIToggleGroup *, int );
  UICheckBox( float xPos, float yPos, const std::string& pText, void (*pClickFunc)(bool,int), int pClickFuncParameter );
  void SetToggleGroup( UIToggleGroup * , int );
  void setText( const std::string& );
  void setState( bool );
  bool getState();
  void setClickFunc( void (*f)( bool, int ),int );

  UIFrame *processLeftClick( float, float );
};
#endif
