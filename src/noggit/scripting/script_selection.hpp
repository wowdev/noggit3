// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/script_chunk.hpp>

#include <math/vector_3d.hpp>

#include <vector>
#include <string>
#include <memory>

class MapChunk;
class World;

namespace noggit
{
  namespace scripting
  {
    class script_context;
    class model_iterator;
    class vert_iterator;
    class tex_iterator;
    class noisemap;

    class selection: public script_object
    {
    public:
      selection( script_context * ctx
               , std::string const& caller
               , math::vector_3d const& point1
               , math::vector_3d const& point2
               );

      std::shared_ptr<noisemap> make_noise(
          float frequency
        , std::string const& algorithm
        , std::string const& seed
        );

      math::vector_3d center();
      math::vector_3d min();
      math::vector_3d max();
      math::vector_3d size();

      std::vector<chunk> chunks_raw();
      std::vector<vert> verts_raw();
      std::vector<tex> textures_raw();
      std::vector<model> models_raw();

      sol::as_table_t<std::vector<chunk>> chunks();
      sol::as_table_t<std::vector<vert>> verts();
      sol::as_table_t<std::vector<tex>> textures();
      sol::as_table_t<std::vector<model>> models();

      void apply();
    
    private:
      World* _world;
      math::vector_3d _center;
      math::vector_3d _min;
      math::vector_3d _max;
      math::vector_3d _size;
    };

    void register_selection(script_context * state);
  } // namespace scripting
} // namespace noggit
