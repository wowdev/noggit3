#ifndef __CHECKBOXUI_H
#define __CHECKBOXUI_H

#include <string>

#include <noggit/UIFrame.h>

#include <noggit/UITexture.h>
#include <noggit/UIText.h>
#include <noggit/UIToggleGroup.h>

class UICheckBox : public UIFrame
{
public:
  typedef UICheckBox* Ptr;

protected:
  UITexture::Ptr check;
  UIText::Ptr text;
  bool checked;
  int id;
  void (*clickFunc)( bool, int );

  UIToggleGroup::Ptr mToggleGroup;

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
