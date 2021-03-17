// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/World.h>
#include <math/vector_3d.hpp>

namespace noggit
{
  namespace scripting
  {
    selection::selection(const char* caller,math::vector_3d const& point1, math::vector_3d const& point2)
    {
      selection sel;
      // TODO: restore
      //sel._world = get_ctx(context, caller)->_world;
      _min = math::vector_3d(
        std::min(point1.x, point2.x),
        std::min(point1.y, point2.y),
        std::min(point1.z, point2.z));

      _max = math::vector_3d(
        std::max(point1.x, point2.x),
        std::max(point1.y, point2.y),
        std::max(point1.z, point2.z));

      _size = _max - _min;
      _center = _min + (_size / 2);
      //_models = model_iterator(sel._world, sel._min, sel._max);
    }

    selection select_origin(math::vector_3d const& origin, float xRadius, float zRadius)
    {
      return selection("select_origin",
               math::vector_3d(origin.x - xRadius, 0, origin.z - zRadius),
               math::vector_3d(origin.x + xRadius, 0, origin.z + zRadius));
    }

    selection select_between(math::vector_3d const& point1, math::vector_3d const& point2)
    {
      return selection("select_between",point1,point2);
    }

    math::vector_3d selection::center() 
    { 
      return _center; 
    }
    math::vector_3d selection::min() 
    { 
      return _min; 
    }
    math::vector_3d selection::max() 
    { 
      return _max; 
    }
    math::vector_3d selection::size() 
    { 
      return _size; 
    }

    void register_selection(sol::state * state, scripting_tool * tool)
    {
      state->new_usertype<selection>("selection"
        , "center", &selection::center
        , "min", &selection::min
        , "max", &selection::max
        , "size", &selection::size
      );
    }
  } // namespace scripting
} // namespace noggit
