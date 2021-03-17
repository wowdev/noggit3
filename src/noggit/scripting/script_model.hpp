// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>

#include <boost/variant.hpp>

class World;

class ModelInstance;
class WMOInstance;

namespace sol {
  class state;
}

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class model
    {
    public:
      model(ModelInstance* model);
      model(WMOInstance* model);
      model() = default;

      math::vector_3d get_pos();
      void set_pos(math::vector_3d& pos);

      math::vector_3d get_rot();
      void set_rot(math::vector_3d& rot);

      float get_scale();
      void set_scale(float scale);

      unsigned get_uid();

      void remove();

      std::string get_filename();
      void replace(const char* filename);

    private:
      boost::variant<ModelInstance*, WMOInstance*> _impl;
    };

    class model_iterator
    {
    public:
      model_iterator(World* world, math::vector_3d min, math::vector_3d max);
      model_iterator() = default;

      bool next();
      void reset();
      void query();
      model get();

    private:
      World* _world;
      math::vector_3d _min;
      math::vector_3d _max;

      char* _models = nullptr;
      model* get_models() { return (model*)_models;}

      bool _initialized = false;
      int _model_index = -1;
      int _models_size = 0;
    };

    void register_model(sol::state * state, scripting_tool * tool);
  } // namespace scripting
} // namespace noggit
