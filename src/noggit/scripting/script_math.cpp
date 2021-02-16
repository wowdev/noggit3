#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <boost/filesystem.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <dukglue.h>

using namespace noggit::scripting;

float noggit::scripting::round(float a1) { return ::round(a1); }
float noggit::scripting::pow(float a1, float a2) { return ::pow(a1, a2); }
float noggit::scripting::log10(float arg) { return ::log10(arg); }
float noggit::scripting::log(float arg) { return ::log(arg); }
float noggit::scripting::ceil(float arg) { return (int) ::ceil(arg); }
float noggit::scripting::floor(float arg) { return (int) ::floor(arg); }
float noggit::scripting::exp(float arg) { return ::exp(arg); }
float noggit::scripting::cbrt(float arg) { return ::cbrt(arg); }
float noggit::scripting::acosh(float arg) { return ::acosh(arg); }
float noggit::scripting::asinh(float arg) { return ::asinh(arg); }
float noggit::scripting::atanh(float arg) { return ::atanh(arg); }
float noggit::scripting::cosh(float arg) { return ::cosh(arg); }
float noggit::scripting::sinh(float arg) { return ::sinh(arg); }
float noggit::scripting::tanh(float arg) { return ::tanh(arg); }
float noggit::scripting::acos(float arg) { return ::acos(arg); }
float noggit::scripting::asin(float arg) { return ::asin(arg); }
float noggit::scripting::atan(float arg) { return ::atan(arg); }
float noggit::scripting::cos(float arg) { return ::cos(arg); }
float noggit::scripting::sin(float arg) { return ::sin(arg); }
float noggit::scripting::tan(float arg) { return ::tan(arg); }
float noggit::scripting::sqrt(float arg) { return ::sqrt(arg); }
float noggit::scripting::abs(float arg) { return ::abs(arg); }

void noggit::scripting::register_misc_functions(duk_context *ctx)
{

    GLUE_FUNCTION(ctx,round);
    GLUE_FUNCTION(ctx,pow);
    GLUE_FUNCTION(ctx,log10);
    GLUE_FUNCTION(ctx,log);
    GLUE_FUNCTION(ctx,ceil);
    GLUE_FUNCTION(ctx,floor);
    GLUE_FUNCTION(ctx,exp);
    GLUE_FUNCTION(ctx,cbrt);
    GLUE_FUNCTION(ctx,acosh);
    GLUE_FUNCTION(ctx,asinh);
    GLUE_FUNCTION(ctx,atanh);
    GLUE_FUNCTION(ctx,cosh);
    GLUE_FUNCTION(ctx,sinh);
    GLUE_FUNCTION(ctx,tanh);
    GLUE_FUNCTION(ctx,acos);
    GLUE_FUNCTION(ctx,asin);
    GLUE_FUNCTION(ctx,atan);
    GLUE_FUNCTION(ctx,cos);
    GLUE_FUNCTION(ctx,sin);
    GLUE_FUNCTION(ctx,tan);
    GLUE_FUNCTION(ctx,sqrt);
    GLUE_FUNCTION(ctx,abs);
}