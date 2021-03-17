// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/script_chunk.hpp>
#include <math/vector_3d.hpp>

#include <vector>
#include <string>

class MapChunk;
class World;

namespace das {
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    class selection
    {
    public:
      selection(const char* caller,math::vector_3d const& point1, math::vector_3d const& point2);
      selection() = default;

      math::vector_3d center();
      math::vector_3d min();
      math::vector_3d max();
      math::vector_3d size();
    
    private:
      World* _world;
      math::vector_3d _center;
      math::vector_3d _min;
      math::vector_3d _max;
      math::vector_3d _size;

      int _chunks_size = 0;

      char* _chunks;
      MapChunk** get_chunks() { return (MapChunk**) _chunks;}
      int _cur_chunk = -1;
      bool _initialized_chunks = false;
    };

    selection select_origin(math::vector_3d const& origin, float xRadius, float zRadius);
    selection select_between(math::vector_3d const& point1, math::vector_3d const& point2);

    void register_selection(sol::state * state, scripting_tool * tool);
  } // namespace scripting
} // namespace noggit
