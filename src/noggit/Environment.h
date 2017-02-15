// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/Selection.h>

#include <boost/optional.hpp>

#include <map>
#include <string>

class Environment
{
public:
  static Environment* getInstance();
  selection_type get_clipboard();
  void set_clipboard(boost::optional<selection_type> entry);
  void clear_clipboard();
  bool is_clipboard();

  // values for areaID painting
  int selectedAreaID;
  std::map<int, math::vector_3d> areaIDColors; // List of all area IDs to draw them with different colors

  bool paintMode;
  int flagPaintMode;
  int screenX;
  int screenY;

  int groundBrushType;

  float minRotation;
  float maxRotation;
  float minTilt;
  float maxTilt;
  float minScale;
  float maxScale;
  bool moveModelToCursorPos;

  bool displayAllWaterLayers;
  int currentWaterLayer;

private:
  Environment();
  static Environment* instance;

  boost::optional<selection_type> clipboard;
};
