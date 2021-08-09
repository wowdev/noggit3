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

    std::shared_ptr<noisemap> selection::make_noise(
        float frequency
      , std::string const& algorithm
      , std::string const& seed)
    {
      return noggit::scripting::make_noise(
          state()
        // padding to ensure we get no rounding errors (should be smaller?)
        , std::floor(_min.x)-2
        , std::floor(_min.z)-2
        , std::ceil(_size.x)+4
        , std::ceil(_size.z)+4
        , frequency
        , algorithm
        , seed
        );
    }

    std::vector<chunk> selection::chunks_raw()
    {
      std::vector<MapChunk*> mapChunks;
      _world->select_all_chunks_between(_min, _max, mapChunks);
      std::vector<chunk> chunks;
      chunks.reserve(mapChunks.size());
      for (auto& chnk : mapChunks) chunks.emplace_back(state(), chnk);
      return chunks;
    }

    std::vector<vert> selection::verts_raw()
    {
      std::vector<vert> verts;
      for (auto chunk : chunks_raw())
      {
        for (int i = 0; i < mapbufsize; ++i)
        {
          auto& v = chunk._chunk->mVertices[i];
          if (v.x >= _min.x && v.x <= _max.x &&
            v.z >= _min.z && v.z <= _max.z)
          {
            verts.emplace_back(state(),chunk._chunk,i);
          }
        }
      }
      return verts;
    }

    std::vector<tex> selection::textures_raw()
    {
      std::vector<tex> texvec;
      for (auto chnk : chunks_raw())
      {
        collect_textures(state(), chnk._chunk, texvec, _min, _max);
      }
      return texvec;
    }

    std::vector<model> selection::models_raw()
    {
      std::vector<model> models;
      collect_models(state(), world(), _min, _max, models);
      return models;
    }

    sol::as_table_t<std::vector<chunk>> selection::chunks()
    {
      return sol::as_table(chunks_raw());
    }

    sol::as_table_t<std::vector<vert>> selection::verts()
    {
      return sol::as_table(verts_raw());
    }

    sol::as_table_t<std::vector<tex>> selection::textures()
    {
      return sol::as_table(textures_raw());
    }

    sol::as_table_t<std::vector<model>> selection::models()
    {
      return sol::as_table(models_raw());
    }

    void selection::apply()
    {
      for (auto& chnk : chunks_raw())
      {
        chnk.apply_all();
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
        , "verts", &selection::verts
        , "tex", &selection::textures
        , "models", &selection::models
        , "chunks", &selection::chunks
        , "make_noise", &selection::make_noise
        );

      state->set_function("select_origin", [state](
          math::vector_3d const& origin
        , float xRadius
        , float zRadius = -1
        )
      {
        if(zRadius <= 0)
        {
          zRadius = xRadius;
        }
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
