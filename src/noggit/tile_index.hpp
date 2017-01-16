// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/MapHeaders.h>

struct tile_index
{
  tile_index(const math::vector_3d& pos) : tile_index(pos.x / TILESIZE, pos.z / TILESIZE) { }
  tile_index(std::size_t tileX, std::size_t tileZ) : x(tileX), z(tileZ)
  {
    assert(x < 64);
    assert(z < 64);
  }

  friend bool operator== (tile_index const& lhs, tile_index const& rhs)
  {
    return std::tie (lhs.x, lhs.z) == std::tie (rhs.x, rhs.z);
  }

  int x;
  int z;
};