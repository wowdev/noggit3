#include <vector>

#include <math/vector_2d.hpp>

class MapChunk;

namespace sol {
  class state;
}

namespace noggit {
  namespace scripting {
    struct tex;
    class scripting_tool;
    class tex_iterator {
      public:
        tex_iterator(
          std::vector<MapChunk*> chunks
          , math::vector_2d const& min
          , math::vector_2d const& max);
        bool next();
        tex get();

      private:
        std::vector<MapChunk*>::iterator _chunk_iter;
        int _tex_iter = -1;
        std::vector<MapChunk*> _chunks;
        math::vector_2d const& _min;
        math::vector_2d const& _max;
    };

    void register_tex_iterator(sol::state * state, scripting_tool * tool);
  }
}