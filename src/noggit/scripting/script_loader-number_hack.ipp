#include <daScript/daScript.h>

#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_filesystem.hpp>
#include <noggit/scripting/script_math.hpp>
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_brush.hpp>
#include <noggit/scripting/script_misc.hpp>
#include <noggit/scripting/script_chunk.hpp>
#include <noggit/scripting/script_model.hpp>
#include <noggit/scripting/script_vert.hpp>
#include <noggit/scripting/script_heap.hpp>

using namespace noggit::scripting;
using namespace das;
namespace ns = noggit::scripting;

/**
 * This file contains a workaround for accepting int32 values in parameters that normally
 * accept floats, since daScript does not convert them normally. This is to cause less frustration for scripters
 * using int literals (10, 20 etc.) when calling functions like: vert(xVal,0,yVal);
 *
 * If/when anyone removes this hack, just keep in mind what functions are
 * registered here (look for <wrapper registry> at the bottom of the file) 
 * and register them in the normal 'script_loader-noggit_module.ipp' file again.
 */

/**
 * Creating your own wrapped numeric wrapped function (allowing ints for float arguments):
 * 1. Import the correct header file above (noggit/scripting/my_module.hpp)
 * 2. Create a static template function with the same name as the wrapped function
 * 3. Call the wrapped function with all the arguments using the "ns::" prefix
 * 4. Register the function name to the bottom of this file with the NUM_X macro (x being the amount of float arguments, 1-3)
 *    (if your function returns a struct, use the NUM_RET_X macro instead)
 *
 * Don't bother doing this in the other direction (allowing floats in int arguments),
 * there are so many more int parameters than floats and it's more likely to cause unexpected behavior.
 * 
 * Finally, to keep all this managable, please don't handle doubles/uints/int64s unless you absolutely have to.
 */

// You can use this one as a reference, it normally takes 3 float arguments, so it uses 3 templates.
// ctrl+f "vert_set_color" to see how it's registered below.
template <typename A, typename B, typename C>
static void vert_set_color(vert &vert, A a, B b, C c)
{
  ns::vert_set_color(vert, a, b, c);
}

template <typename A>
static void vert_set_alpha(vert &vert, int index, A alpha)
{
  ns::vert_set_alpha(vert, index, alpha);
}

template <typename A>
static void vert_set_water(vert &vert, int type, A height)
{
  ns::vert_set_water(vert, type, height);
}

template <typename A>
static void vert_set_height(vert &vert, A y)
{
  ns::vert_set_height(vert, y);
}

template <typename A>
static void vert_add_height(vert &vert, A y)
{
  ns::vert_add_height(vert, y);
}

template <typename A>
static void vert_sub_height(vert &vert, A y)
{
  ns::vert_sub_height(vert, y);
}

template <typename A>
static void tex_set_alpha(tex &tex, int index, A alpha)
{
  ns::tex_set_alpha(tex, index, alpha);
}

template <typename A, typename B>
static float rand_float(random &rand, A low, B high)
{
  return ns::rand_float(rand, low, high);
}

template <typename A>
static void model_set_scale(model &model, A scale)
{
  ns::model_set_scale(model, scale);
}

template <typename A, typename B, typename C>
static math::vector_3d vec(A x, B y, C z)
{
  return ns::vec(x, y, z);
}

template <typename A>
static void add_m2(char const *filename, math::vector_3d const &pos, A scale, math::vector_3d const &rotation)
{
  ns::add_m2(filename, pos, scale, rotation);
}
template <typename A, typename B, typename C>
static float lerp(A from, B to, C amount)
{
  return ns::lerp(from, to, amount);
}

template <typename A>
static math::vector_3d rotate_2d(math::vector_3d const &point, math::vector_3d const &origin, A angleDeg)
{
  return ns::rotate_2d(point, origin, angleDeg);
}

template <typename A>
static float abs(A arg)
{
  return ns::abs(arg);
}

template <typename A, typename B, typename C>
static void add_float_param(char const *path, A min, B max, C def, int zeros)
{
  return ns::add_float_param(path, min, max, def, zeros);
}

template <typename A, typename B>
static selection select_origin(math::vector_3d const &origin, A xRadius, B zRadius)
{
  return ns::select_origin(origin, xRadius, zRadius);
}

template <typename A>
static noisemap make_noise_size(int start_x, int start_y, int width, int height, A frequency, char const *algorithm, char const *seed, das::Context *ctx)
{
  return ns::make_noise_size(start_x, start_y, width, height, frequency, algorithm, seed, ctx);
}

template <typename A>
static noisemap make_noise_selection(selection const &sel, A frequency, int padding, char const *algorithm, char const *seed, das::Context *ctx)
{
  return ns::make_noise_selection(sel, frequency, padding, algorithm, seed, ctx);
}

void register_hack_functions(das::Module *module, das::ModuleLibrary &lib)
{

#define NUM_BASE(name, side_effect, ...) \
  addExtern<DAS_BIND_FUN((::name<__VA_ARGS__>))>(*module, lib, #name, SideEffects::side_effect, #name);

#define NUM_RET_BASE(name, side_effect, ...) \
  addExtern<DAS_BIND_FUN((::name<__VA_ARGS__>)), SimNode_ExtFuncCallAndCopyOrMove>(*module, lib, #name, SideEffects::side_effect, #name);

  // the hall of shame...
#define NUM_1(name, side_effect)    \
  NUM_BASE(name, side_effect, int); \
  NUM_BASE(name, side_effect, float);

#define NUM_1_RET(name, side_effect)    \
  NUM_RET_BASE(name, side_effect, int); \
  NUM_RET_BASE(name, side_effect, float);

#define NUM_2(name, side_effect)           \
  NUM_BASE(name, side_effect, int, int);   \
  NUM_BASE(name, side_effect, int, float); \
  NUM_BASE(name, side_effect, float, int); \
  NUM_BASE(name, side_effect, float, float);

#define NUM_2_RET(name, side_effect)           \
  NUM_RET_BASE(name, side_effect, int, int);   \
  NUM_RET_BASE(name, side_effect, int, float); \
  NUM_RET_BASE(name, side_effect, float, int); \
  NUM_RET_BASE(name, side_effect, float, float);

#define NUM_3(name, side_effect)                  \
  NUM_BASE(name, side_effect, int, int, int);     \
  NUM_BASE(name, side_effect, int, int, float);   \
  NUM_BASE(name, side_effect, int, float, int);   \
  NUM_BASE(name, side_effect, int, float, float); \
  NUM_BASE(name, side_effect, float, int, int);   \
  NUM_BASE(name, side_effect, float, int, float); \
  NUM_BASE(name, side_effect, float, float, int); \
  NUM_BASE(name, side_effect, float, float, float);

#define NUM_3_RET(name, side_effect)                  \
  NUM_RET_BASE(name, side_effect, int, int, int);     \
  NUM_RET_BASE(name, side_effect, int, int, float);   \
  NUM_RET_BASE(name, side_effect, int, float, int);   \
  NUM_RET_BASE(name, side_effect, int, float, float); \
  NUM_RET_BASE(name, side_effect, float, int, int);   \
  NUM_RET_BASE(name, side_effect, float, int, float); \
  NUM_RET_BASE(name, side_effect, float, float, int); \
  NUM_RET_BASE(name, side_effect, float, float, float);

  // <wrapper registry>
  NUM_1(vert_set_water, worstDefault);
  NUM_3(vert_set_color, worstDefault);
  NUM_2(rand_float, worstDefault);
  NUM_1(vert_set_height, worstDefault);
  NUM_1(vert_add_height, worstDefault);
  NUM_1(vert_sub_height, worstDefault);
  NUM_1(vert_set_alpha, worstDefault);

  NUM_1(model_set_scale, worstDefault);
  NUM_3_RET(vec, worstDefault);
  NUM_1(add_m2, worstDefault);
  NUM_3(lerp, worstDefault);
  NUM_1_RET(rotate_2d, worstDefault);

  NUM_1(abs, worstDefault);
  NUM_2_RET(select_origin, worstDefault);
  NUM_3(add_float_param, worstDefault);
  NUM_1_RET(make_noise_size, worstDefault);
  NUM_1_RET(make_noise_selection, worstDefault);
}