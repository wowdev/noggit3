#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <dukglue.h>

using namespace noggit::scripting;

noggit::scripting::script_random::script_random(unsigned seed)
{
    _engine = std::mt19937(seed);
}

noggit::scripting::script_random::script_random(std::string seed)
{
    _engine = std::mt19937(std::hash<std::string>()(seed));
}

noggit::scripting::script_random::script_random()
{
    _engine = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

int noggit::scripting::script_random::get_int32(int low, int high)
{
    return std::uniform_int_distribution<int>(low, high)(_engine); 
}


unsigned noggit::scripting::script_random::get_uint32(unsigned low, unsigned high)
{
    return std::uniform_int_distribution<unsigned>(low, high)(_engine);
}


unsigned long noggit::scripting::script_random::get_uint64(unsigned long low, unsigned long high)
{
    return std::uniform_int_distribution<unsigned long>(low, high)(_engine);
}

long noggit::scripting::script_random::get_int64(long low, long high)
{
    return std::uniform_int_distribution<long>(low, high)(_engine);
}

double noggit::scripting::script_random::get_double(double low, double high)
{
    return std::uniform_real_distribution<double>(low, high)(_engine);
}

double noggit::scripting::script_random::get_float(float low, float high)
{
    return std::uniform_real_distribution<float>(low, high)(_engine);
}

std::shared_ptr<script_random> noggit::scripting::random_from_seed(std::string seed)
{
    return std::make_shared<script_random>(seed);
}

std::shared_ptr<script_random> noggit::scripting::random_from_time()
{
    return std::make_shared<script_random>();
}

void noggit::scripting::register_random_functions(duk_context* ctx)
{
    GLUE_METHOD(ctx, script_random,get_int32);
    GLUE_METHOD(ctx, script_random,get_uint32);
    //GLUE_METHOD(ctx, script_random,get_uint64);
    //GLUE_METHOD(ctx, script_random,get_int64);
    GLUE_METHOD(ctx, script_random,get_double);
    GLUE_METHOD(ctx, script_random,get_float);

    GLUE_FUNCTION(ctx,random_from_seed);
    GLUE_FUNCTION(ctx,random_from_time);
}