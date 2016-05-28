// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/UIFrame.h>

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
