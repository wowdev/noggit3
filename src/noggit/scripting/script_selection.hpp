// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/script_chunk.hpp>
#include <math/vector_3d.hpp>

#include <vector>
#include <string>
#include <memory>

class MapChunk;
class World;

namespace sol {
  class state;
}

namespace noggit
{
  namespace scripting
  {
    class model_iterator;
    class vert_iterator;
    class tex_iterator;
    class noisemap;

    class selection
    {
    public:
      selection(World* world, std::string const& caller,math::vector_3d const& point1, math::vector_3d const& point2);
      selection() = default;

      std::shared_ptr<noisemap> make_noise(float frequency, std::string const& algorithm, std::string const& seed);

      math::vector_3d center();
      math::vector_3d min();
      math::vector_3d max();
      math::vector_3d size();

      std::shared_ptr<model_iterator> get_model_iterator();
      std::shared_ptr<vert_iterator> get_vert_iterator();
      std::shared_ptr<tex_iterator> get_tex_iterator();
    
    private:
      std::shared_ptr<std::vector<MapChunk*>> _chunks = nullptr;
      std::shared_ptr<std::vector<MapChunk*>> get_chunks();
      World* _world;
      math::vector_3d _center;
      math::vector_3d _min;
      math::vector_3d _max;
      math::vector_3d _size;
    };

    void register_selection(script_context * state);
  } // namespace scripting
} // namespace noggit
