// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/CloseWindow.h>
#include <noggit/MapView.h>

class UISaveCurrentWarning : public UICloseWindow
{
private:
  static const int winWidth = 640;
  static const int winHeight = 120;
  MapView *_MapView;
public:
	UISaveCurrentWarning(MapView *mapView);
  void resize();
  void exitNow();
};
