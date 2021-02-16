#include <noggit/scripting/script_vec.hpp>
#include <noggit/scripting/script_loader.hpp>

using namespace noggit::scripting;

noggit::scripting::script_vec::script_vec(math::vector_3d vec) 
: _vec(vec) {}

noggit::scripting::script_vec::script_vec(float x, float y, float z) 
: _vec(x,y,z) {}

noggit::scripting::script_vec::script_vec() 
: _vec(0,0,0) {}

float noggit::scripting::script_vec::x() { return _vec.x; }
float noggit::scripting::script_vec::y() { return _vec.y; }
float noggit::scripting::script_vec::z() { return _vec.z; }

std::shared_ptr<script_vec> noggit::scripting::script_vec::add(float x, float y, float z)
{
    return std::make_shared<script_vec>(_vec.x+x,_vec.y+y,_vec.z+z); 
}

std::shared_ptr<script_vec> noggit::scripting::vec(float x, float y, float z) 
{ 
    return std::make_shared<script_vec>(x,y,z); 
}

void noggit::scripting::register_vec_functions(duk_context* ctx)
{
    GLUE_FUNCTION(ctx,vec);

    GLUE_METHOD(ctx,script_vec,x);
    GLUE_METHOD(ctx,script_vec,y);
    GLUE_METHOD(ctx,script_vec,z);
    GLUE_METHOD(ctx,script_vec,add);
}