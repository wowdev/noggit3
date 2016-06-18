#pragma once

#include <boost/optional/optional.hpp>
#include <math/vector_3d.h>
namespace math{
  struct ray  {    vector_3d origin;    vector_3d direction;  };
  boost::optional<float> intersect_bounds(ray const& _ray, vector_3d const& _min, vector_3d const& _max);  boost::optional<float> intersect_triangle(ray const& _ray, vector_3d const& _v0, vector_3d const& _v1, vector_3d const& _v2);}

