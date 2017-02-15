// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Selection.h>
#include <noggit/Misc.h>

#include <boost/optional.hpp>

#include <map>
#include <string>

struct random_color : math::vector_4d
{
  random_color()
    : math::vector_4d ( misc::randfloat(0.0f, 1.0f)
                      , misc::randfloat(0.0f, 1.0f)
                      , misc::randfloat(0.0f, 1.0f)
                      , 0.7f
                      )
  {}
};

class Environment
{
public:
  static Environment* getInstance();
  selection_type get_clipboard();
  void set_clipboard(boost::optional<selection_type> entry);
  void clear_clipboard();
  bool is_clipboard();

  // values for areaID painting
  std::map<int, random_color> areaIDColors; // List of all area IDs to draw them with different colors

  bool paintMode;
  int flagPaintMode;

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
