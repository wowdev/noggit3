#include <iostream>
#include <fstream>
#include <stack>

#include <dukglue.h>
#include <boost/filesystem.hpp>

#include <noggit/Log.h>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/scripting/scripting_tool.hpp>

#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_setup.hpp>
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/script_printf.hpp>
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_vec.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_filesystem.hpp>

using namespace noggit::scripting;

static duk_context *ctx = nullptr;
static std::vector<std::string> scripts;
static int selected_script = -1;

static void handle_error(int value)
{
    if (value)
    {
        throw script_exception((std::string(duk_safe_to_string(ctx,-1))).c_str());
    }
}

std::string filename(std::string filename)
{
    return boost::filesystem::path(filename).filename().string();
}

std::string dirname(std::string filename)
{
    return boost::filesystem::path(filename).parent_path().string();
}

std::string resolve(std::string path)
{
    return boost::filesystem::relative(
        boost::filesystem::absolute(
            boost::filesystem::path(path)), 
            boost::filesystem::path("./"))
            .string();
}

void register_script_functions(duk_context* ctx)
{
    register_setup_functions(ctx);
    register_vec_functions(ctx);
    register_misc_functions(ctx);
    register_image_functions(ctx);
    register_noise_functions(ctx);
    register_random_functions(ctx);
    register_print_functions(ctx);
    register_context_functions(ctx);
    register_selection_functions(ctx);
    register_filesystem_functions(ctx);
}

#define CLICK_EVENT(type)                      \
    static bool has_##type##_handler = false;  \
    duk_ret_t on_##type(duk_context *ctx)      \
    {                                          \
        has_##type##_handler = true;           \
        duk_dup(ctx, 0);                       \
        duk_put_global_string(ctx, #type);     \
        return 0;                              \
    }                                          \
    void send_##type(script_context *script)   \
    {                                          \
        if (has_##type##_handler)              \
        {                                      \
            duk_get_global_string(ctx, #type); \
            dukglue_push(ctx, script);         \
            duk_int_t rc = duk_pcall(ctx, 1);  \
            handle_error(rc);                  \
            duk_pop(ctx);                      \
        }                                      \
    }

CLICK_EVENT(left_click)
CLICK_EVENT(left_hold)
CLICK_EVENT(left_release)

CLICK_EVENT(right_click)
CLICK_EVENT(right_hold)
CLICK_EVENT(right_release)

/** 
 * Require Stuff 
 * 
 * This segment can be useful for anyone trying to use duktape with CommonJS  
 */

struct ExportsObject {};

std::map<std::string, ExportsObject> exports;
std::vector<std::string> module_stack;

ExportsObject* __get_exports(std::string key)
{
    return &exports[key];
}

inline bool ends_with(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

ExportsObject* require(std::string path)
{
    auto parent = module_stack.back();
    auto out = resolve((boost::filesystem::path(dirname(parent)) / boost::filesystem::path(path)).string());
    std::replace(out.begin(), out.end(), '\\', '/');

    // the first element is just the root and will be duplicate, so ignore it.
    if (module_stack.size() > 1)
    {
        auto index = std::find(module_stack.begin()+1, module_stack.end(), out);
        if (index != module_stack.end())
        {
            std::string error_msg = "Circular dependency in module: " + out+"\nRequire stack (grows down): \n      ";
            index = module_stack.begin()+1;
            while (index != module_stack.end())
            {
                error_msg += (*index);
                if (*index == out) error_msg += " << seen here";
                error_msg += "\n    ->";
                index++;
            }
            error_msg += out + " << and again here (error)\n";
            throw script_exception(error_msg.c_str());
        }
    }
    
    auto exports_object = &(exports[out] = ExportsObject());
    module_stack.push_back(out);
    // isolate the script context
    auto fullstr = "(function(){var exports = __get_exports('" + out + "'); " + read_file(out + ".js") + ";}());";
    int res = duk_peval_string(ctx,fullstr.c_str());
    handle_error(res);
    module_stack.pop_back();
    return exports_object;
}

int noggit::scripting::load_scripts()
{
    exports.clear();
    module_stack.clear();

    auto tool = get_cur_tool();
    // TODO: error
    if (tool == nullptr)
    {
        return -1;
    }
    std::string lastName = "";
    if (selected_script != -1)
        lastName = scripts[selected_script];

    LogDebug << "Loading scripts" << std::endl;
    if (ctx != nullptr)
    {
        duk_destroy_heap(ctx);
    }
    scripts.clear();

    ctx = duk_create_heap_default();
    duk_push_global_object(ctx);

    // Events
    TAPE_FUNCTION(ctx, on_left_click, 1);
    TAPE_FUNCTION(ctx, on_left_hold, 1);
    TAPE_FUNCTION(ctx, on_left_release, 1);
    TAPE_FUNCTION(ctx, on_right_click, 1);
    TAPE_FUNCTION(ctx, on_right_hold, 1);
    TAPE_FUNCTION(ctx, on_right_release, 1);

    GLUE_FUNCTION(ctx,require);
    GLUE_FUNCTION(ctx,__get_exports);

    register_script_functions(ctx);
    tool->clearLog();

    using namespace boost::filesystem;
    if(boost::filesystem::exists("scripts") && boost::filesystem::is_directory("scripts"))
    {
        boost::filesystem::recursive_directory_iterator dir("scripts"), end;
        while(dir != end)
        {
            if(!boost::filesystem::is_directory(dir->path()) && ends_with(dir->path().string(),".js"))
            {
                auto path = dir->path().string();
                path = path.substr(0,path.size()-3);
                module_stack.push_back(path);
                require("./"+filename(path));
                module_stack.pop_back();
            }
            boost::system::error_code ec;
            dir.increment(ec);
            if(ec)
            {
                throw script_exception(("Error accessing " + dir->path().string() + "::" + ec.message() + "\n").c_str());
            }
        }
    }

    tool->clearDescription();
    if (selected_script != -1)
    {
        auto it = std::find(scripts.begin(), scripts.end(), lastName);
        if (it != scripts.end())
        {
            selected_script = -1;
            select_script(it - scripts.begin());
            return selected_script;
        }
    }
    selected_script = -1;
    select_script(0);
    return selected_script;
}

void noggit::scripting::select_script(int index)
{
    auto tool = get_cur_tool();
    if (tool == nullptr || index == selected_script || index >= scripts.size())
    {
        return;
    }

    tool->removeScriptWidgets();

    selected_script = index;
    tool->clearDescription();
    tool->clearLog();

    has_left_click_handler = false;
    has_left_hold_handler = false;
    has_left_release_handler = false;
    has_right_click_handler = false;
    has_right_hold_handler = false;
    has_right_release_handler = false;

    auto name = scripts[index];
    duk_get_global_string(ctx, name.c_str());
    duk_int_t rc = duk_pcall(ctx, 0);
    handle_error(rc);
}

std::vector<std::string> *noggit::scripting::get_scripts()
{
    return &scripts;
}