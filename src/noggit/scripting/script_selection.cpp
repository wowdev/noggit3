// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <noggit/MapView.h>
#include <noggit/World.h>
#include <math/vector_3d.hpp>

namespace noggit
{
  namespace scripting
  {
    selection::selection(script_context * ctx, std::string const&,math::vector_3d const& point1, math::vector_3d const& point2)
      : script_object(ctx)
      , _world(ctx->world())
    {
      _min = math::vector_3d(
        std::min(point1.x, point2.x),
        std::min(point1.y, point2.y),
        std::min(point1.z, point2.z));

      _max = math::vector_3d(
        std::max(point1.x, point2.x),
        std::max(point1.y, point2.y),
        std::max(point1.z, point2.z));

      _size = _max - _min;
      _center = _min + (_size / 2);
    }

    math::vector_3d selection::center() 
    { 
      return _center; 
    }
    math::vector_3d selection::min() 
    { 
      return _min; 
    }
    math::vector_3d selection::max() 
    { 
      return _max; 
    }
    math::vector_3d selection::size() 
    { 
      return _size; 
    }

    std::shared_ptr<std::vector<MapChunk*>> selection::get_chunks()
    {
      if(_chunks==nullptr)
      {
        _chunks = std::make_shared<std::vector<MapChunk*>>();
        _world->select_all_chunks_between(_min,_max,*_chunks);
      }
      return _chunks;
    }

    std::shared_ptr<model_iterator> selection::get_model_iterator()
    {
      return std::make_shared<model_iterator>(state(),_min, _max);
    }

    std::shared_ptr<vert_iterator> selection::get_vert_iterator()
    {
      return std::make_shared<vert_iterator>(state(),get_chunks(), _min, _max);
    }

    std::shared_ptr<tex_iterator> selection::get_tex_iterator()
    {
      return std::make_shared<tex_iterator>(state(),get_chunks(), _min, _max);
    }

    std::shared_ptr<chunk_iterator> selection::get_chunk_iterator()
    {
      return std::make_shared<chunk_iterator>(state(),get_chunks());
    }

    void selection::apply_noise(std::shared_ptr<noisemap> noise, float ratio)
    {
      auto verts = this->get_vert_iterator();
      while(verts->next())
      {
        auto vert = verts->get();
      }
    }

    std::shared_ptr<noisemap> selection::make_noise(
        float frequency
      , std::string const& algorithm
      , std::string const& seed)
    {
      return noggit::scripting::make_noise(
          state()
        , _min.x
        , _min.z
        , _size.x
        , _size.z
        , frequency
        , algorithm
        , seed
        );
    }

    void selection::apply()
    {
      for(auto& chnk: *get_chunks())
      {
        chunk(state(),chnk).apply_all();
      }
    }

    void register_selection(script_context* state)
    {
      state->new_usertype<selection>("selection"
        , "center", &selection::center
        , "min", &selection::min
        , "max", &selection::max
        , "size", &selection::size
        , "apply", &selection::apply
        , "verts", &selection::get_vert_iterator
        , "tex", &selection::get_tex_iterator
        , "models", &selection::get_model_iterator
        , "chunks", &selection::get_chunk_iterator
        );

      state->set_function("select_origin", [state](
          math::vector_3d const& origin
        , float xRadius
        , float zRadius
        ) 
      {
        return std::make_shared<selection>(state, "select_origin",
          math::vector_3d(origin.x - xRadius, 0, origin.z - zRadius),
          math::vector_3d(origin.x + xRadius, 0, origin.z + zRadius));
      });

      state->set_function("select_between", [state](
          math::vector_3d const& point1
        , math::vector_3d const& point2
        )
      {
        return std::make_shared<selection>(state, "select_between",
          point1,point2);
      });
    }
  } // namespace scripting
} // namespace noggit
