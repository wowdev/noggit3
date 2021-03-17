// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <noggit/World.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/ui/ObjectEditor.h>

#include <util/visit.hpp>

#include <boost/algorithm/string/predicate.hpp>

namespace noggit
{
  namespace scripting
  {
    model::model (ModelInstance* model)
      : _impl (model)
    {}

    model::model (WMOInstance* model)
      : _impl (model)
    {}

    math::vector_3d model_get_pos(model const& model)
    {
      return util::visit (model._impl, [] (auto x) { return x->pos; });
    }

    void model_set_pos(model& model, math::vector_3d& pos)
    {
      return util::visit (model._impl, [&] (auto x) { x->pos = pos; });
    }

    math::vector_3d model_get_rot(model const& model)
    {
      return math::vector_3d {util::visit (model._impl, [] (auto x) { return x->dir; })};
    }

    void model_set_rot(model& model, math::vector_3d& rot)
    {
      return util::visit (model._impl, [&] (auto x) { x->dir = math::degrees::vec3 {rot}; });
    }

    float model_get_scale(model const& model)
    {
      return util::visit ( model._impl
                         , [] (ModelInstance const* as_m2) {
                             return as_m2->scale;
                           }
                         , [] (WMOInstance const*) {
                             return 1.0f;
                           }
                         );
    }

    void model_set_scale(model& model, float scale)
    {
      return util::visit ( model._impl
                         , [&] (ModelInstance* as_m2) {
                             as_m2->scale = scale;
                           }
                         , [] (WMOInstance*) {}
                         );
    }

    unsigned model_get_uid(model const& model)
    {
      return util::visit ( model._impl
                         , [] (ModelInstance const* as_m2) {
                             return as_m2->uid;
                           }
                         , [] (WMOInstance const* as_wmo) {
                             return as_wmo->mUniqueID;
                           }
                         );
          }

    std::string model_get_filename(model const& model)
    {
      return util::visit ( model._impl
                      , [] (ModelInstance const* as_m2) {
                          return as_m2->model->filename;
                        }
                      , [] (WMOInstance const* as_wmo) {
                          return as_wmo->wmo->filename;
                        }
                      );
    }

    void model_remove(model& model)
    {
      std::vector<selection_type> type;

      util::visit (model._impl, [&] (auto x) { type.emplace_back (x); });

      // TODO: fix
      //get_ctx(context, "model_remove")->_world->delete_models(type);
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
        // TODO: Fix
        //model._impl = get_ctx(ctx, "model_replace")
        //  ->_world->addWMO(filename, model_get_pos(model), math::degrees::vec3 {model_get_rot(model)});
      }
      else
      {
        auto params = object_paste_params();
        // TODO: Fix
        //model._impl = get_ctx(ctx, "model_replace")
        //  ->_world->addM2(filename, model_get_pos(model), model_get_scale(model), math::degrees::vec3 {model_get_rot(model)}, &params);
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
