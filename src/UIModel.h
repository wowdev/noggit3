#ifndef MODELUI_H
#define MODELUI_H

#include "UIFrame.h"

class Model;

class UIModel : public UIFrame
{
public:
  UIModel( float x, float y, float width, float height );

  void render() const;
  void setModel( Model* setModel );

private:
  Model* model;

  GLuint fbo;
  GLuint modelTexture;
  GLuint depthBuffer;

  void drawFBO() const;
  void drawTexture() const;
};
#endif
