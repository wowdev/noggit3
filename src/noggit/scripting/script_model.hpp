// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>

#include <boost/variant.hpp>

class World;

class ModelInstance;
class WMOInstance;

namespace das {
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    struct model
    {
      model(ModelInstance* model);
      model(WMOInstance* model);
      model() = default;

      boost::variant<ModelInstance*, WMOInstance*> _impl;
    };

    struct model_iterator
    {
      model_iterator(World* world, math::vector_3d min, math::vector_3d max);
      model_iterator() = default;

      World* _world;
      math::vector_3d _min;
      math::vector_3d _max;

      char* _models = nullptr;
      model* get_models() { return (model*)_models;}

      bool _initialized = false;
      int _model_index = -1;
      int _models_size = 0;

      bool next();
      void reset_itr();
      void query();
      model get();
    };

    math::vector_3d model_get_pos(model const& model);
    void model_set_pos(model& model, math::vector_3d& pos);

    math::vector_3d model_get_rot(model const& model);
    void model_set_rot(model& model, math::vector_3d& rot);

    float model_get_scale(model const& model);
    void model_set_scale(model& model, float scale);

    unsigned model_get_uid(model const& model);

    void model_remove(model& model, das::Context* context);

    std::string model_get_filename(model const& model);
    void model_replace(model& model, const char* filename);
  } // namespace scripting
} // namespace noggit
