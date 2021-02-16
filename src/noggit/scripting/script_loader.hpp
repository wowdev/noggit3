// licensed under GNU General Public License (version 3).

#pragma once

#include <vector>
#include <string>

#include <noggit/scripting/script_context.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/Log.h>
#include <math/vector_3d.hpp>

// Registers a duktape-style function
#define TAPE_FUNCTION(ctx, fun, args) \
    duk_push_c_function(ctx, fun, args); \
    duk_put_prop_string(ctx, -2, #fun);

// Registers a normal c++ function
#define GLUE_FUNCTION(ctx, fun) \
    dukglue_register_function(ctx, fun, #fun);

// Registers a c++ method
#define GLUE_METHOD(ctx, obj, fun) \
    dukglue_register_method(ctx, &obj::fun, #fun);

/**
 * script_loader.hpp
 * 
 * This file manages the current script context and all event listeners.
 * 
 */
namespace noggit
{
    namespace scripting
    {
        class script_context;

        int load_scripts();
        void select_script(int index);
        std::vector<std::string>* get_scripts();
    } // namespace scripting
} // namespace noggit

void send_left_click(noggit::scripting::script_context*);
void send_left_hold(noggit::scripting::script_context*);
void send_left_release(noggit::scripting::script_context*);

void send_right_click(noggit::scripting::script_context*);
void send_right_hold(noggit::scripting::script_context*);
void send_right_release(noggit::scripting::script_context*);