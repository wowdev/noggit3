#ifndef MODELUI_H
#define MODELUI_H

#include <string>

#include "UIFrame.h"

class Model;

class UIModel : public UIFrame
{
public:
  UIModel( float x, float y, float width, float height );

  void render() const;
  void setModel(const std::string &name);

private:
  Model* model;

  GLuint fbo;
  GLuint modelTexture;
  GLuint depthBuffer;

  void drawFBO() const;
  void drawTexture() const;
};
#endif
