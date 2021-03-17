#include <noggit/scripting/script_vert_iterator.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_vert.hpp>
#include <sol/sol.hpp>

namespace noggit {
  namespace scripting {
    vert_iterator::vert_iterator(
      std::vector<MapChunk*> chunks
      , math::vector_2d const& min
      , math::vector_2d const& max)
      : _chunks(chunks)
      , _min(min)
      , _max(max)
      {}

      bool vert_iterator::next()
      {
        if(_chunk_iter==_chunks.end())
        {
          return false;
        }

        ++_vert_iter;
        while(_vert_iter < 145)
        {
          auto& vert = (*_chunk_iter)->mVertices[_vert_iter];
          if (vert.x <= _min.x || vert.x >= _max.x ||
            vert.z <= _min.y || vert.z >= _max.y)
          {
            ++_vert_iter;
          }
          else
          {
            break;
          }
        }

        if(_vert_iter >= 145)
        {
          ++_chunk_iter;
          _vert_iter = -1;
          return next();
        }
      }

      vert vert_iterator::get()
      {
        return vert(*_chunk_iter,_vert_iter);
      }

      void register_vert_iterator(sol::state * state, scripting_tool * tool)
      {
        state->new_usertype<vert_iterator>("vert_iterator"
          , "next", &vert_iterator::next
          , "get", &vert_iterator::get
        );
      }
  }
}