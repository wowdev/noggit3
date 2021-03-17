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

    void register_tex(sol::state * state, scripting_tool * tool);
  }
}