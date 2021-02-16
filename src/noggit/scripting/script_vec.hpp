#pragma once

#include <memory>

#include <math/vector_3d.hpp>
#include <dukglue.h>

struct duk_hthread;

namespace noggit {
    namespace scripting {
        struct script_vec {
            script_vec(math::vector_3d vec);
            script_vec(float x, float y, float z);
            script_vec();

            float x();
            float y();
            float z();

            std::shared_ptr<script_vec> add(float x, float y, float z);

            math::vector_3d _vec;
        };

        std::shared_ptr<script_vec> vec(float x, float y, float z);
        void register_vec_functions(duk_hthread* ctx);
    }
}