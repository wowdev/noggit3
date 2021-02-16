#pragma once

#include <memory>
#include <string>
#include <boost/filesystem.hpp>

struct script_vec;
class script_random;
class script_image;
class script_noise_generator;
struct duk_hthread;

namespace noggit {
    namespace scripting {

        // math stuff
        float round(float a1);
        float pow(float a1, float a2);
        float log10(float arg);
        float log(float arg);
        float ceil(float arg);
        float floor(float arg);
        float exp(float arg);
        float cbrt(float arg);
        float acosh(float arg);
        float asinh(float arg);
        float atanh(float arg);
        float cosh(float arg);
        float sinh(float arg);
        float tanh(float arg);
        float acos(float arg);
        float asin(float arg);
        float atan(float arg);
        float cos(float arg);
        float sin(float arg);
        float tan(float arg);
        float sqrt(float arg);
        float abs(float arg);
        
        // Registry function (not available from script)
        void register_misc_functions(duk_hthread *ctx);
    }
}