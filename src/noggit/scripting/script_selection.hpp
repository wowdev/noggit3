// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

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

namespace noggit
{
  namespace scripting
  {
    struct script_selection
    {
      script_selection(World* world, math::vector_3d& origin, float radius_x, float radius_y);
      script_selection(World* world, math::vector_3d& point1, math::vector_3d& point2);
      script_selection();

      World* _world;
      math::vector_3d _center;
      math::vector_3d _min;
      math::vector_3d _max;
      math::vector_3d _size;

      int _chunks_size = 0;
      MapChunk** _chunks = nullptr;
      int _cur_chunk = -1;
      bool _initialized_chunks = false;

      script_model_iterator _models;
    };

    script_selection make_selector();
    void select_origin(script_selection& sel, math::vector_3d& origin, float xRadius, float zRadius);
    void select_between(script_selection& sel, math::vector_3d const& point1, math::vector_3d const& point2);

    bool sel_next_chunk(script_selection& sel);
    script_chunk sel_get_chunk(script_selection& sel);
    void sel_reset_chunk_itr(script_selection& sel);

    bool sel_next_model(script_selection& sel);
    script_model sel_get_model(script_selection& sel);
    void sel_reset_model_itr(script_selection& sel);
    void sel_requery_models(script_selection& sel);

    math::vector_3d sel_center(script_selection& sel);
    math::vector_3d sel_min(script_selection& sel);
    math::vector_3d sel_max(script_selection& sel);
    math::vector_3d sel_size(script_selection& sel);
  } // namespace scripting
} // namespace noggit
