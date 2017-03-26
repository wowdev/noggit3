// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>

#include <boost/optional.hpp>

#include <map>
#include <string>

class Environment
{
public:
  static Environment* getInstance();

  float minRotation;
  float maxRotation;
  float minTilt;
  float maxTilt;
  float minScale;
  float maxScale;

  noggit::bool_toggle_property displayAllWaterLayers;
  int currentWaterLayer;

private:
  Environment();
  static Environment* instance;
};
