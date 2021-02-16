#pragma once

struct duk_hthread;

namespace noggit {
    namespace scripting {
        duk_ret_t print(duk_hthread *ctx);
        void register_print_functions(duk_hthread *ctx);
    }
}