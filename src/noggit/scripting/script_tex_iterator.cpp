#include <noggit/scripting/script_tex_iterator.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_vert.hpp>
#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    tex_iterator::tex_iterator(
      std::vector<MapChunk*> chunks
      , math::vector_2d const& min
      , math::vector_2d const& max)
      : _chunks(chunks)
      , _min(min)
      , _max(max)
      {}

      bool tex_iterator::next()
      {
        if(_chunk_iter==_chunks.end())
        {
          return false;
        }

        ++_tex_iter;
        while(_tex_iter < 4096)
        {
          // TODO: Implement skipping
          break;
        }

        if(_tex_iter >= 4096)
        {
          ++_chunk_iter;
          _tex_iter = -1;
          return next();
        }
      }

      tex tex_iterator::get()
      {
        return tex(*_chunk_iter,_tex_iter);
      }

      void register_tex_iterator(sol::state * state, scripting_tool * tool)
      {
        state->new_usertype<tex_iterator>("tex_iterator"
          , "next", &tex_iterator::next
          , "get", &tex_iterator::get
        );
      }
  }
}