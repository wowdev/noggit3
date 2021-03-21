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

#include <sol/sol.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace noggit
{
  namespace scripting
  {
    model::model (script_context * ctx, ModelInstance* model)
      : script_object(ctx)
      , _impl (model)
    {}

    model::model (script_context * ctx, WMOInstance* model)
      : script_object(ctx)
      , _impl (model)
    {}

    math::vector_3d model::get_pos()
    {
      return util::visit (_impl, [] (auto x) { return x->pos; });
    }

    void model::set_pos(math::vector_3d& pos)
    {
      return util::visit (_impl, [&] (auto x) { x->pos = pos; });
    }

    math::vector_3d model::get_rot()
    {
      return math::vector_3d {util::visit (_impl, [] (auto x) { return x->dir; })};
    }

    void model::set_rot(math::vector_3d& rot)
    {
      return util::visit (_impl, [&] (auto x) { x->dir = math::degrees::vec3 {rot}; });
    }

    float model::get_scale()
    {
      return util::visit ( _impl
                         , [] (ModelInstance const* as_m2) {
                             return as_m2->scale;
                           }
                         , [] (WMOInstance const*) {
                             return 1.0f;
                           }
                         );
    }

    void model::set_scale(float scale)
    {
      return util::visit ( _impl
                         , [&] (ModelInstance* as_m2) {
                             as_m2->scale = scale;
                           }
                         , [] (WMOInstance*) {}
                         );
    }

    unsigned model::get_uid()
    {
      return util::visit ( _impl
                         , [] (ModelInstance const* as_m2) {
                             return as_m2->uid;
                           }
                         , [] (WMOInstance const* as_wmo) {
                             return as_wmo->mUniqueID;
                           }
                         );
          }

    std::string model::get_filename()
    {
      return util::visit ( _impl
                      , [] (ModelInstance const* as_m2) {
                          return as_m2->model->filename;
                        }
                      , [] (WMOInstance const* as_wmo) {
                          return as_wmo->wmo->filename;
                        }
                      );
    }

    void model::remove()
    {
      std::vector<selection_type> type;
      util::visit (_impl, [&] (auto x) { type.emplace_back (x); });
      // TODO: fix
      //get_ctx(context, "model_remove")->_world->delete_models(type);
    }

    void model::replace(std::string const& filename)
    {
      if (get_filename() == filename)
      {
        return;
      }

      remove();

      if (boost::ends_with(filename, ".wmo"))
      {
        _impl = 
          world()->addWMO(filename, get_pos(), math::degrees::vec3 {get_rot()});
      }
      else
      {
        auto params = object_paste_params();
        _impl =
          world()->addM2(filename, get_pos(), get_scale(), math::degrees::vec3 {get_rot()}, &params);
      }
    }

    model_iterator::model_iterator(
          script_context * ctx
        , math::vector_3d min
        , math::vector_3d max
        )
        : script_object(ctx)
        , _min(min), _max(max) {}

    void model_iterator::query()
    {
      _models.clear();
      world()->for_each_m2_instance([&](ModelInstance& mod) {
        if (mod.pos.x >= _min.x && mod.pos.x <= _max.x 
         && mod.pos.z >= _min.z && mod.pos.z <= _max.z)
        {
          _models.push_back(model(state(),&mod));
        }
      });
      world()->for_each_wmo_instance([&](WMOInstance& mod) {
        if (mod.pos.x >= _min.x && mod.pos.x <= _max.x 
         && mod.pos.z >= _min.z && mod.pos.z <= _max.z)
        {
          _models.push_back(model(state(),&mod));
        }
      });

      _initialized = true;
      reset();
    }

    void model_iterator::reset()
    {
      _model_index = -1;
    }

    bool model_iterator::next()
    {
      if (!_initialized)
      {
        query();
      }

      if (_model_index >= _models.size())
      {
        return false;
      }

      ++_model_index;
      return _model_index < _models.size();
    }

    model model_iterator::get()
    {
      if(_model_index >= _models.size())
      {
        throw script_exception(
          "model_iterator#get",
          "accessing invalid model: iterator is done");
      }

      return _models[_model_index];
    }

    void register_model(script_context * state)
    {
      state->new_usertype<model>("model"
        , "get_pos", &model::get_pos
        , "set_pos", &model::set_pos
        , "get_rot", &model::get_rot
        , "set_rot", &model::set_rot
        , "get_scale", &model::get_scale
        , "set_scale", &model::set_scale
        , "get_uid", &model::get_uid
        , "remove", &model::remove
        , "get_filename", &model::get_filename
        , "replace", &model::replace
      );

      state->new_usertype<model_iterator>("model_iterator"
        , "next", &model_iterator::next
        , "reset", &model_iterator::reset
        , "get", &model_iterator::get
      );
    }
  } // namespace scripting
} // namespace noggit
