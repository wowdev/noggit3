#ifndef MODELUI_H
#define MODELUI_H

#include "UIFrame.h"

class Model;

class UIModel : public UIFrame
{
protected:
  Model* model;
public:
  UIModel( float x, float y, float width, float height );
  void render() const;
  void setModel( Model* setModel );
};
#endif
