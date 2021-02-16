#include <noggit/scripting/script_setup.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <dukglue.h>

using namespace noggit::scripting;

duk_ret_t noggit::scripting::add_script(duk_context *ctx)
{
    auto str = std::string(duk_to_string(ctx, 0));
    LogDebug << "Registering script " << str << std::endl;
    duk_dup(ctx, 1);
    duk_put_global_string(ctx, str.c_str());
    get_scripts()->push_back(str);
    return 0;
}

void noggit::scripting::add_desc(std::string desc)
{
    if(get_cur_tool())
    {
        get_cur_tool()->addDescription(desc);
    }
}

double_holder *noggit::scripting::param_double(std::string name, double min, double max, double def)
{
    return get_cur_tool() ? get_cur_tool()->addDouble(name, min, max, def) : nullptr;
}

int_holder *noggit::scripting::param_int(std::string name, int min, int max, int def)
{
    return get_cur_tool() ? get_cur_tool()->addInt(name, min, max, def) : nullptr;
}

string_holder *noggit::scripting::param_string(std::string name, std::string def)
{
    return get_cur_tool() ? get_cur_tool()->addString(name, def) : nullptr;
}

bool_holder *noggit::scripting::param_bool(std::string name, bool def)
{
    return get_cur_tool() ? get_cur_tool()->addBool(name, def) : nullptr;
}

void noggit::scripting::register_setup_functions(duk_context* ctx)
{
    GLUE_METHOD(ctx,string_holder,get);
    GLUE_METHOD(ctx,int_holder,get);
    GLUE_METHOD(ctx,double_holder,get);
    GLUE_METHOD(ctx,bool_holder,get);

    TAPE_FUNCTION(ctx, add_script,2);
    GLUE_FUNCTION(ctx, add_desc);
    GLUE_FUNCTION(ctx, param_double);
    GLUE_FUNCTION(ctx, param_int);
    GLUE_FUNCTION(ctx, param_string);
    GLUE_FUNCTION(ctx, param_bool);
}