// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/MapHeaders.h>

struct tile_index
{
  tile_index(const math::vector_3d& pos) : tile_index(std::floor(pos.x / TILESIZE), std::floor(pos.z / TILESIZE)) { }
  tile_index(std::size_t tileX, std::size_t tileZ) : x(tileX), z(tileZ) { }

  friend bool operator== (tile_index const& lhs, tile_index const& rhs)
  {
    return std::tie (lhs.x, lhs.z) == std::tie (rhs.x, rhs.z);
  }

  bool is_valid() const
  {
    // x and z are unsigned so negative signed int value are positive and > 63
    return x < 64 && z < 64;
  }

  float dist(tile_index const& other) const
  {
    return (math::vector_3d(x, 0.f, z) - math::vector_3d(other.x, 0.f, other.z)).length();
  }

  std::size_t x;
  std::size_t z;
};
