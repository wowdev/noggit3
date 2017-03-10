// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/camera.hpp>
#include <noggit/map_horizon.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/Window.h>

class Menu;
class World;


class UIMinimapWindow : public UIWindow
{
private:
  float borderwidth;
  float tilesize;
  World* map;
  UIText::Ptr cursor_position;
  noggit::map_horizon::minimap _minimap;
  noggit::camera* _camera;

public:
  explicit UIMinimapWindow(World* setMap, noggit::camera*);
  UIFrame* processLeftClick(float mx, float my);
  virtual void mouse_moved (float, float) override;
  void resize();
  void render() const;
};
