// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <boost/algorithm/string/predicate.hpp>
#include <noggit/scripting/script_model.hpp>
#include <noggit/World.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/ui/ObjectEditor.h>

using namespace noggit::scripting;

noggit::scripting::script_model::script_model(ModelInstance *model)
    : _model((void *)model), _is_wmo(false) {}

noggit::scripting::script_model::script_model(WMOInstance *model)
    : _model((void *)model), _is_wmo(true) {}

static WMOInstance *wmo(script_model &model)
{
    return (WMOInstance *)model._model;
}

static ModelInstance *m2(script_model &model)
{
    return (ModelInstance *)model._model;
}

math::vector_3d noggit::scripting::model_get_pos(script_model &model)
{
    if (model._is_wmo)
    {
        return wmo(model)->pos;
    }
    else
    {
        return m2(model)->pos;
    }
}

void noggit::scripting::model_set_pos(script_model &model, math::vector_3d &pos)
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

math::vector_3d noggit::scripting::model_get_rot(script_model &model)
{
    if (model._is_wmo)
    {
        return wmo(model)->dir;
    }
    else
    {
        return m2(model)->dir;
    }
}

void noggit::scripting::model_set_rot(script_model &model, math::vector_3d &rot)
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

float noggit::scripting::model_get_scale(script_model &model)
{
    if (model._is_wmo)
    {
        return 1;
    }
    else
    {
        return m2(model)->scale;
    }
}

void noggit::scripting::model_set_scale(script_model &model, float scale)
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

unsigned noggit::scripting::model_get_uid(script_model &model)
{
    if (model._is_wmo)
    {
        return wmo(model)->mUniqueID;
    }
    else
    {
        return m2(model)->uid;
    }
}

const char *noggit::scripting::model_get_filename(script_model &model)
{
    if (model._is_wmo)
    {
        return wmo(model)->wmo->filename.c_str();
    }
    else
    {
        return m2(model)->model->filename.c_str();
    }
}

void noggit::scripting::model_remove(script_model &model)
{
    std::vector<selection_type> type;
    if (model._is_wmo)
    {
        type.push_back((WMOInstance *)model._model);
    }
    else
    {
        type.push_back((ModelInstance *)model._model);
    }
    get_ctx()->_world->delete_models(type);
}

void noggit::scripting::model_set_filename(script_model &model, const char *filename)
{
    if (model_get_filename(model) == filename)
    {
        return;
    }

    model_remove(model);

    if (boost::ends_with(filename, ".wmo"))
    {
        auto wmo = get_ctx()->_world->addWMO(filename, model_get_pos(model), model_get_rot(model));
        model._model = wmo;
        model._is_wmo = true;
    }
    else
    {
        auto params = object_paste_params();
        auto m2 = get_ctx()->_world->addM2(filename, model_get_pos(model), model_get_scale(model), model_get_rot(model), &params);
        model._model = m2;
        model._is_wmo = false;
    }
}

noggit::scripting::script_model_iterator::script_model_iterator(World *world, math::vector_3d min, math::vector_3d max)
    : _world(world), _min(min), _max(max) {}

void noggit::scripting::script_model_iterator::query()
{
    _models.clear();
    _world->for_each_m2_instance([&](ModelInstance &model) {
        if (model.pos.x >= _min.x && model.pos.x <= _max.x && model.pos.z >= _min.z && model.pos.z <= _max.z)
        {
            _models.push_back(script_model(&model));
        }
    });
    _world->for_each_wmo_instance([&](WMOInstance &model) {
        if (model.pos.x >= _min.x && model.pos.x <= _max.x && model.pos.z >= _min.z && model.pos.z <= _max.z)
        {
            _models.push_back(script_model(&model));
        }
    });
    _initialized = true;
    reset_itr();
}

void noggit::scripting::script_model_iterator::reset_itr()
{
    _model_index = -1;
}

bool noggit::scripting::script_model_iterator::next()
{
    if (!_initialized)
    {
        query();
    }

    if (_model_index >= int(_models.size()))
    {
        return false;
    }

    ++_model_index;
    return _model_index < _models.size();
}

script_model noggit::scripting::script_model_iterator::get()
{
    return _models[_model_index];
}