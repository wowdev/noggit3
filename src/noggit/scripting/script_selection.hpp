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

struct TextureIndex
{
  short indices[36];
};

namespace das {
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    struct script_selection
    {
      script_selection() = default;

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

      script_model_iterator _models;
    };

    script_selection make_selector();
    void select_origin(script_selection& sel, math::vector_3d const& origin, float xRadius, float zRadius);
    void select_between(script_selection& sel, math::vector_3d const& point1, math::vector_3d const& point2);

    bool sel_next_chunk(script_selection& sel, das::Context* ctx);
    script_chunk sel_get_chunk(script_selection& sel);
    void sel_reset_chunk_itr(script_selection& sel);

    bool sel_next_model(script_selection& sel, das::Context * ctx);
    script_model sel_get_model(script_selection& sel);
    void sel_reset_model_itr(script_selection& sel);
    void sel_requery_models(script_selection& sel, das::Context* ctx);

    math::vector_3d sel_center(script_selection const& sel);
    math::vector_3d sel_min(script_selection const& sel);
    math::vector_3d sel_max(script_selection const& sel);
    math::vector_3d sel_size(script_selection const& sel);
  } // namespace scripting
} // namespace noggit
