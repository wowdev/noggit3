#pragma once

#include <math/vector_2d.hpp>
#include <math/vector_3d.hpp>

struct Vec3D : math::vector_3d
{
  using math::vector_3d::vector_3d;
  Vec3D (math::vector_3d x) : math::vector_3d (x) {}
  Vec3D() : math::vector_3d() {}
};

struct Vec2D : math::vector_2d
{
  using math::vector_2d::vector_2d;
  Vec2D (math::vector_2d x) : math::vector_2d (x) {}
  Vec2D() : math::vector_2d() {}
};
