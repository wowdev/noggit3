#pragma once

#include <noggit/scripting/script_object.hpp>

#include <math/vector_3d.hpp>

class MapChunk;

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_context;

    class tex: public script_object
    {
    public:
      tex(script_context * ctx, MapChunk* chunk, int index);
      void set_alpha(int index, float alpha);
      float get_alpha(int index);
      math::vector_3d get_pos_2d();
    
    private:
      MapChunk* _chunk;
      int _index;
    };

    class tex_iterator : public script_object {
      public:
        tex_iterator( script_context * ctx
                    , std::shared_ptr<std::vector<MapChunk*>> chunks
                    , math::vector_3d const& min
                    , math::vector_3d const& max
                    );
        bool next();
        tex get();

      private:
        std::vector<MapChunk*>::iterator _chunk_iter;
        int _tex_iter = -1;
        std::shared_ptr<std::vector<MapChunk*>> _chunks;
        math::vector_3d const& _min;
        math::vector_3d const& _max;
    };

    void register_tex(script_context * state);
  }
}