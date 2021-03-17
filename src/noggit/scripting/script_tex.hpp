#pragma once

#include <math/vector_3d.hpp>

class MapChunk;

namespace sol {
  class state;
}

namespace noggit {
  namespace scripting {
    class scripting_tool;

    class tex
    {
    public:
      tex(MapChunk* chunk, int index);
      tex() = default;

      void set_alpha(int index, float alpha);
      float get_alpha(int index);
      math::vector_3d get_pos_2d();
    
    private:
      MapChunk* _chunk;
      int _index;
    };

    class tex_iterator {
      public:
        tex_iterator(
          std::shared_ptr<std::vector<MapChunk*>> chunks
          , math::vector_3d const& min
          , math::vector_3d const& max);
        bool next();
        tex get();

      private:
        std::vector<MapChunk*>::iterator _chunk_iter;
        int _tex_iter = -1;
        std::shared_ptr<std::vector<MapChunk*>> _chunks;
        math::vector_3d const& _min;
        math::vector_3d const& _max;
    };

    void register_tex(sol::state * state, scripting_tool * tool);
  }
}