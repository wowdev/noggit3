// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <noggit/World.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/ui/ObjectEditor.h>
#include <boost/algorithm/string/predicate.hpp>

namespace noggit
{
  namespace scripting
  {
    static const WMOInstance* wmo_const(model const& model)
    {
      return (const WMOInstance*)model._model;
    }

    static const ModelInstance* m2_const(model const& model)
    {
      return (const ModelInstance*)model._model;
    }

    static WMOInstance* wmo(model& model)
    {
      return (WMOInstance*)model._model;
    }

    static ModelInstance* m2(model& model)
    {
      return (ModelInstance*)model._model;
    }

    model::model(ModelInstance*model)
      : _model((void*)model), _is_wmo(false) {}

    model::model(WMOInstance*model)
      : _model((void*)model), _is_wmo(true) {}

    math::vector_3d model_get_pos(model const& model)
    {
      if (model._is_wmo)
      {
        return wmo_const(model)->pos;
      }
      else
      {
        return m2_const(model)->pos;
      }
    }

    void model_set_pos(model& model, math::vector_3d& pos)
    {
      if (model._is_wmo)
      {
        wmo(model)->pos = pos;
      }
      else
      {
        m2(model)->pos = pos;
      }
    }

    math::vector_3d model_get_rot(model const& model)
    {
      if (model._is_wmo)
      {
        return wmo_const(model)->dir;
      }
      else
      {
        return m2_const(model)->dir;
      }
    }

    void model_set_rot(model& model, math::vector_3d& rot)
    {
      if (model._is_wmo)
      {
        wmo(model)->dir = rot;
      }
      else
      {
        m2(model)->dir = rot;
      }
    }

    float model_get_scale(model const& model)
    {
      if (model._is_wmo)
      {
        return 1;
      }
      else
      {
        return m2_const(model)->scale;
      }
    }

    void model_set_scale(model& model, float scale)
    {
      if (model._is_wmo)
      {
        return;
      }
      else
      {
        m2(model)->scale = scale;
      }
    }

    unsigned model_get_uid(model const& model)
    {
      if (model._is_wmo)
      {
        return wmo_const(model)->mUniqueID;
      }
      else
      {
        return m2_const(model)->uid;
      }
    }

    std::string model_get_filename(model const& model)
    {
      if (model._is_wmo)
      {
        return wmo_const(model)->wmo->filename;
      }
      else
      {
        return m2_const(model)->model->filename;
      }
    }

    void model_remove(model& model)
    {
      std::vector<selection_type> type;
      if (model._is_wmo)
      {
        type.push_back((WMOInstance*)model._model);
      }
      else
      {
        type.push_back((ModelInstance*)model._model);
      }
      get_ctx("model_remove")->_world->delete_models(type);
    }

    void model_replace(model& model, char const* filename)
    {
      if(filename==nullptr)
      {
        throw script_exception(
          "model_replace",
          "empty filename (in call to model_replace)");
      }

      if (model_get_filename(model) == filename)
      {
        return;
      }

      model_remove(model);

      if (boost::ends_with(filename, ".wmo"))
      {
        auto wmo = get_ctx("model_replace")
          ->_world->addWMO(filename, model_get_pos(model), model_get_rot(model));
        model._model = wmo;
        model._is_wmo = true;
      }
      else
      {
        auto params = object_paste_params();
        auto m2 = get_ctx("model_replace")
          ->_world->addM2(filename, model_get_pos(model), model_get_scale(model), model_get_rot(model),& params);
        model._model = m2;
        model._is_wmo = false;
      }
    }

    model_iterator::model_iterator(World* world, math::vector_3d min, math::vector_3d max)
      : _world(world), _min(min), _max(max) {}

    void model_iterator::query()
    {
      std::vector<model> models;

      _world->for_each_m2_instance([&](ModelInstance& mod) {
        if (mod.pos.x >= _min.x && mod.pos.x <= _max.x && mod.pos.z >= _min.z && mod.pos.z <= _max.z)
        {
          models.push_back(model(&mod));
        }
      });
      _world->for_each_wmo_instance([&](WMOInstance& mod) {
        if (mod.pos.x >= _min.x && mod.pos.x <= _max.x && mod.pos.z >= _min.z && mod.pos.z <= _max.z)
        {
          models.push_back(model(&mod));
        }
      });

      _models_size = models.size();
      size_t size = models.size() * sizeof(model);
      // TODO: Leak
      _models = new char[size];
      memcpy(get_models(), models.data(), size);
      _initialized = true;
      reset_itr();
    }

    void model_iterator::reset_itr()
    {
      _model_index = -1;
    }

    bool model_iterator::next()
    {
      if (!_initialized)
      {
        query();
      }

      if (_model_index >= int(_models_size))
      {
        return false;
      }

      ++_model_index;
      return _model_index < _models_size;
    }

    model model_iterator::get()
    {
      if(_model_index >= int(_models_size))
      {
        throw script_exception(
          "model_iterator#get",
          "accessing invalid model: iterator is done");
      }

      return get_models()[_model_index];
    }
  } // namespace scripting
} // namespace noggit
