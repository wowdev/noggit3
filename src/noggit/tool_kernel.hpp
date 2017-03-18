// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>

#include <boost/optional.hpp>

#include <functional>

class MapChunk;

namespace noggit
{
  struct chunk_kernel
  {
    math::vector_3d pos;
    float radius;
    std::function<bool (MapChunk*)> fun;
    bool recalc_normals_after_function = false;
  };

  struct chunk_stencil_kernel
  {
    math::vector_3d pos;
    float radius;
    std::function<bool (MapChunk*, std::function<boost::optional<float> (float, float)>)> fun;
    bool recalc_normals_after_function = false;
  };
}
