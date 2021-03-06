// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <math/vector_3d.hpp>

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
    struct script_model
    {
      script_model(ModelInstance* model);
      script_model(WMOInstance* model);
      script_model() = default;

      void* _model = nullptr;
      bool _is_wmo = false;
    };

    struct script_model_iterator
    {
      script_model_iterator(World* world, math::vector_3d min, math::vector_3d max);
      script_model_iterator() = default;

      World* _world;
      math::vector_3d _min;
      math::vector_3d _max;

      char* _models = nullptr;
      script_model* get_models() { return (script_model*)_models;}

      bool _initialized = false;
      int _model_index = -1;
      int _models_size = 0;

      bool next(das::Context * ctx);
      void reset_itr();
      void query(das::Context* ctx);
      script_model get();
    };

    math::vector_3d model_get_pos(script_model const& model);
    void model_set_pos(script_model& model, math::vector_3d& pos);

    math::vector_3d model_get_rot(script_model const& model);
    void model_set_rot(script_model& model, math::vector_3d& rot);

    float model_get_scale(script_model const& model);
    void model_set_scale(script_model& model, float scale);

    unsigned model_get_uid(script_model const& model);

    void model_remove(script_model& model);

    const char* model_get_filename(script_model const& model, das::Context* ctx);
    void model_set_filename(script_model& model, const char* filename, das::Context* ctx);
  } // namespace scripting
} // namespace noggit
