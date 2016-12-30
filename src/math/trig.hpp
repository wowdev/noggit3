// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cmath>

namespace math
{
  struct radians;

  struct degrees
  {
    explicit degrees (float x) : _ (x) {}
    degrees (radians);

    float _;
  };

  struct radians
  {
    explicit radians (float x) : _ (x) {}
    radians (degrees);

    float _;
  };

  inline degrees::degrees (radians x) : _ (x._ * 180.0f / M_PI) {}
  inline radians::radians (degrees x) : _ (x._ * M_PI / 180.0f) {}

  inline float sin (radians x)
  {
    return std::sin (x._);
  }
  inline float cos (radians x)
  {
    return std::cos (x._);
  }
}
