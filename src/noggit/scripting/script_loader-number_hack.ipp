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

/**
 * This file contains a workaround for accepting int32 values in parameters that normally
 * accept floats, since daScript does not convert them normally. This is to cause less frustration for scripters
 * using int literals (10, 20 etc.) when calling functions like: vert(xVal,0,yVal);
 *
 * If/when anyone removes this hack, just keep in mind what functions are
 * registered here (look for <Wrapper Registry> at the bottom of the file) 
 * and register them in the normal 'script_loader-noggit_module.ipp' file again.
 */

/**
 * Creating your own wrapped numeric wrapped function (allowing ints for float arguments):
 * 1. Import the correct header file above (noggit/scripting/my_module.hpp)
 * 2. Create a static template function with the same name as the wrapped function
 * 3. Call the wrapped function with all the arguments using the "noggit::scripting::" prefix
 * 4. Register the function name to the bottom of this file with the FUNC_NUM_X macro (x being the amount of float arguments, 1-3)
 *    (if your function returns a struct, use the FUNC_NUM_RETCLASS_X macro instead)
 *
 * Don't bother doing this in the other direction (allowing floats in int arguments),
 * there are so many more int parameters than floats and it's more likely to cause unexpected behavior.
 * 
 * Finally, to keep all this managable, please don't handle doubles/uints/int64s unless you absolutely have to.
 */

namespace wraps {
  // You can use this one as a reference, it normally takes 3 float arguments, so it uses 3 templates.
  // ctrl+f "vert_set_color" to see how it's registered below.
  template <typename A, typename B, typename C>
  void vert_set_color(noggit::scripting::vert& vert, A a, B b, C c)
  {
    noggit::scripting::vert_set_color(vert, a, b, c);
  }

  template <typename A>
  void vert_set_alpha(noggit::scripting::vert& vert, int index, A alpha)
  {
    noggit::scripting::vert_set_alpha(vert, index, alpha);
  }

  template <typename A>
  void vert_set_water(noggit::scripting::vert& vert, int type, A height)
  {
    noggit::scripting::vert_set_water(vert, type, height);
  }

  template <typename A>
  void vert_set_height(noggit::scripting::vert& vert, A y)
  {
    noggit::scripting::vert_set_height(vert, y);
  }

  template <typename A>
  void vert_add_height(noggit::scripting::vert& vert, A y)
  {
    noggit::scripting::vert_add_height(vert, y);
  }

  template <typename A>
  void vert_sub_height(noggit::scripting::vert& vert, A y)
  {
    noggit::scripting::vert_sub_height(vert, y);
  }

  template <typename A>
  void tex_set_alpha(noggit::scripting::tex& tex, int index, A alpha)
  {
    noggit::scripting::tex_set_alpha(tex, index, alpha);
  }

  template <typename A, typename B>
  float rand_float(noggit::scripting::random &rand, A low, B high)
  {
    return noggit::scripting::rand_float(rand, low, high);
  }

  template <typename A>
  void model_set_scale(noggit::scripting::model& model, A scale)
  {
    noggit::scripting::model_set_scale(model, scale);
  }

  template <typename A, typename B, typename C>
  math::vector_3d vec(A x, B y, C z)
  {
    return noggit::scripting::vec(x, y, z);
  }

  template <typename A>
  void add_m2(char const *filename, math::vector_3d const &pos, A scale, math::vector_3d const &rotation)
  {
    noggit::scripting::add_m2(filename, pos, scale, rotation);
  }
  template <typename A, typename B, typename C>
  float lerp(A from, B to, C amount)
  {
    return noggit::scripting::lerp(from, to, amount);
  }

  template <typename A>
  math::vector_3d rotate_2d(math::vector_3d const &point, math::vector_3d const &origin, A angleDeg)
  {
    return noggit::scripting::rotate_2d(point, origin, angleDeg);
  }

  template <typename A>
  float abs(A arg)
  {
    return noggit::scripting::abs(arg);
  }

  template <typename A, typename B, typename C>
  void add_float_param(char const *path, A min, B max, C def, int zeros, das::Context* context)
  {
    return noggit::scripting::add_float_param(path, min, max, def, zeros, context);
  }

  template <typename A, typename B>
  noggit::scripting::selection select_origin(math::vector_3d const &origin, A xRadius, B zRadius)
  {
    return noggit::scripting::select_origin(origin, xRadius, zRadius);
  }

  template <typename A>
  noggit::scripting::noisemap make_noise_size(int start_x, int start_y, int width, int height, A frequency, char const *algorithm, char const *seed, das::Context *ctx)
  {
    return noggit::scripting::make_noise_size(start_x, start_y, width, height, frequency, algorithm, seed, ctx);
  }

  template <typename A>
  noggit::scripting::noisemap make_noise_selection(noggit::scripting::selection const& sel, A frequency, int padding, char const *algorithm, char const *seed, das::Context *ctx)
  {
    return noggit::scripting::make_noise_selection(sel, frequency, padding, algorithm, seed, ctx);
  }
}

void register_hack_functions(das::Module *module, das::ModuleLibrary &lib)
{

#define FUNC_NUM_BASE(name_in, name_out, side_effect, ...) \
  das::addExtern<DAS_BIND_FUN((name_in<__VA_ARGS__>))>(*module, lib, name_out, das::SideEffects::side_effect, name_out);

#define FUNC_NUM_RETCLASS_BASE(name_in,name_out, side_effect, ...) \
  das::addExtern<DAS_BIND_FUN((name_in<__VA_ARGS__>)), das::SimNode_ExtFuncCallAndCopyOrMove>(*module, lib, name_out, das::SideEffects::side_effect, name_out);

  // the hall of shame...
#define FUNC_NUM_1(name_in, name_out, side_effect)    \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int); \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float);

#define FUNC_NUM_1_RETCLASS(name_in, name_out, side_effect)    \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int); \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float);

#define FUNC_NUM_2(name_in, name_out, side_effect)           \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int, int);   \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int, float); \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float, int); \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float, float);

#define FUNC_NUM_2_RETCLASS(name_in, name_out, side_effect)           \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int, int);   \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int, float); \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float, int); \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float, float);

#define FUNC_NUM_3(name_in, name_out, side_effect)                  \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int, int, int);     \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int, int, float);   \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int, float, int);   \
  FUNC_NUM_BASE(name_in, name_out, side_effect, int, float, float); \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float, int, int);   \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float, int, float); \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float, float, int); \
  FUNC_NUM_BASE(name_in, name_out, side_effect, float, float, float);

#define FUNC_NUM_3_RETCLASS(name_in, name_out, side_effect)                  \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int, int, int);     \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int, int, float);   \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int, float, int);   \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, int, float, float); \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float, int, int);   \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float, int, float); \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float, float, int); \
  FUNC_NUM_RETCLASS_BASE(name_in, name_out, side_effect, float, float, float);

  /**
   * <Wrapper Registry>
   * - Register functions similarly to FUNC/FUNC_RETCLASS
   * - The number is the amount of templates the function has.
   * - Every combination of int/float will be registered.
   * - The maximum supported amount of parameters is 3. 
   *   Create a new FUNC_NUM_4/FUNC_NUM_4_RETCLASS if you need more
   */
  FUNC_NUM_3(wraps::vert_set_color, "vert_set_color", worstDefault);
  FUNC_NUM_1(wraps::vert_set_water, "vert_set_water", worstDefault);
  FUNC_NUM_1(wraps::vert_set_height, "vert_set_height", worstDefault);
  FUNC_NUM_1(wraps::vert_add_height, "vert_add_height", worstDefault);
  FUNC_NUM_1(wraps::vert_sub_height, "vert_sub_height", worstDefault);
  FUNC_NUM_1(wraps::vert_set_alpha, "vert_set_alpha", worstDefault);
  FUNC_NUM_1(wraps::model_set_scale, "model_set_scale", worstDefault);
  FUNC_NUM_1(wraps::add_m2, "add_m2", worstDefault);
  FUNC_NUM_1(wraps::abs, "abs", worstDefault);
  FUNC_NUM_2(wraps::rand_float, "rand_float", worstDefault);
  FUNC_NUM_3(wraps::lerp, "lerp", worstDefault);
  FUNC_NUM_3(wraps::add_float_param, "add_float_param", worstDefault);

  FUNC_NUM_1_RETCLASS(wraps::rotate_2d, "rotate_2d", worstDefault);
  FUNC_NUM_1_RETCLASS(wraps::make_noise_size, "make_noise_size", worstDefault);
  FUNC_NUM_1_RETCLASS(wraps::make_noise_selection, "make_noise_selection", worstDefault);
  FUNC_NUM_2_RETCLASS(wraps::select_origin, "select_origin", worstDefault);
  FUNC_NUM_3_RETCLASS(wraps::vec, "vec", worstDefault);
}