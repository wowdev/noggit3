// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_heap.hpp>
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
    struct selection
    {
      selection() = default;

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

      model_iterator _models;
    };

    selection select_origin(math::vector_3d const& origin, float xRadius, float zRadius);
    selection select_between(math::vector_3d const& point1, math::vector_3d const& point2);

    bool sel_next_chunk(selection& sel, das::Context* ctx);
    chunk sel_get_chunk(selection& sel);
    void sel_reset_chunk_itr(selection& sel);

    bool sel_next_model(selection& sel, das::Context * ctx);
    model sel_get_model(selection& sel);
    void sel_reset_model_itr(selection& sel);
    void sel_requery_models(selection& sel, das::Context* ctx);

    math::vector_3d sel_center(selection const& sel);
    math::vector_3d sel_min(selection const& sel);
    math::vector_3d sel_max(selection const& sel);
    math::vector_3d sel_size(selection const& sel);
  } // namespace scripting
} // namespace noggit
