#pragma once

#include <math/vector_2d.hpp>
#include <noggit/MapChunk.h>

class MapChunk;
namespace sol {
  class state;
}

namespace noggit {
  namespace scripting {
    struct vert;
    class scripting_tool;

    class vert_iterator {
      public:
        vert_iterator(
          std::vector<MapChunk*> chunks
          , math::vector_2d const& min
          , math::vector_2d const& max);
        bool next();
        vert get();
    private:
      std::vector<MapChunk*>::iterator _chunk_iter;
      int _vert_iter = -1;
      std::vector<MapChunk*> _chunks;
      math::vector_2d const& _min;
      math::vector_2d const& _max;
    };

    void register_vert_iterator(sol::state * state, scripting_tool * tool);
  }
}