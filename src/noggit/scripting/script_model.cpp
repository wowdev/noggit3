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
#include <boost/algorithm/string/case_conv.hpp>

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
      return util::visit ( _impl
                         , [&, this] (ModelInstance * as_m2) {
                            this->world()->updateTilesModel(as_m2, model_update::remove);
                            as_m2->pos = pos;
                            as_m2->recalcExtents();
                            this->world()->updateTilesModel(as_m2, model_update::add);
                         }
                         , [&, this] (WMOInstance * as_wmo) {
                           this->world()->updateTilesWMO(as_wmo, model_update::remove);
                           as_wmo->pos = pos;
                           as_wmo->recalcExtents();
                           this->world()->updateTilesWMO(as_wmo, model_update::add);
                         }
      );
    }

    math::vector_3d model::get_rot()
    {
      return math::vector_3d {util::visit (_impl, [] (auto x) { return x->dir; })};
    }

    void model::set_rot(math::vector_3d& rot)
    {
      math::degrees::vec3 dir = math::degrees::vec3{ rot };
      return util::visit ( _impl
                         , [dir, this] (ModelInstance * as_m2) {
                            this->world()->updateTilesModel(as_m2, model_update::remove);
                            as_m2->dir = dir;
                            as_m2->recalcExtents();
                            this->world()->updateTilesModel(as_m2, model_update::add);
                         }
                         , [dir, this] (WMOInstance * as_wmo) {
                           this->world()->updateTilesWMO(as_wmo, model_update::remove);
                           as_wmo->dir = dir;
                           as_wmo->recalcExtents();
                           this->world()->updateTilesWMO(as_wmo, model_update::add);
                         });
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
                         , [scale, this] (ModelInstance * as_m2) {
                            this->world()->updateTilesModel(as_m2, model_update::remove);
                            as_m2->scale = scale;
                            as_m2->recalcExtents();
                            this->world()->updateTilesModel(as_m2, model_update::add);
                         }
                         , [] (WMOInstance *) {}
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

    bool model::has_filename(std::string const& name)
    {
      std::string copy = std::string(name);
      boost::to_lower(copy);
      std::replace(copy.begin(),copy.end(),'\\','/');
      return copy == get_filename();
    }

    void model::remove()
    {
      std::vector<selection_type> type;
      util::visit (_impl, [&] (auto x) { type.emplace_back (x); });
      world()->delete_models(type);
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

    void collect_models(
        script_context * ctx
      , World * world
      , math::vector_3d const& min
      , math::vector_3d const& max
      , std::vector<model> & vec
    )
    {
      world->for_each_m2_instance([&](ModelInstance& mod) {
        if (mod.pos.x >= min.x && mod.pos.x <= max.x
          && mod.pos.z >= min.z && mod.pos.z <= max.z)
        {
          vec.push_back(model(ctx, &mod));
        }
      });
      world->for_each_wmo_instance([&](WMOInstance& mod) {
        if (mod.pos.x >= min.x && mod.pos.x <= max.x
          && mod.pos.z >= min.z && mod.pos.z <= max.z)
        {
          vec.push_back(model(ctx, &mod));
        }
      });
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
        , "has_filename", &model::has_filename
        , "replace", &model::replace
      );
    }
  } // namespace scripting
} // namespace noggit
