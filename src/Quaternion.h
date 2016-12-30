#pragma once

#include <cmath>

#include <math/quaternion.hpp>
#include <math/vector_4d.hpp>
#include <Vec3D.h>

struct Vec4D : math::vector_4d
{
  using math::vector_4d::vector_4d;
  Vec4D (math::vector_4d x) : math::vector_4d (x) {}
  Vec4D() : math::vector_4d() {}
};


struct Quaternion : math::quaternion
{
  using math::quaternion::quaternion;
  Quaternion (math::quaternion x) : math::quaternion (x) {}
  Quaternion() : math::quaternion() {}
};

//! \note In WoW 2.0+ Blizzard is now storing rotation data in 16bit
//! values instead of 32bit. I don't really understand why as its only
//! a very minor saving in model sizes and adds extra overhead in
//! processing the models. Need this structure to read the data into.
struct PackedQuaternion : math::packed_quaternion
{
  using math::packed_quaternion::packed_quaternion;
  PackedQuaternion (math::packed_quaternion x) : math::packed_quaternion (x) {}
  PackedQuaternion() : math::packed_quaternion() {}
};
