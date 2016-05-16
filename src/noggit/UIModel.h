// UIModel.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

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
