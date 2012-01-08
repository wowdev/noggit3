#ifndef __MATH_BOUNDED_NEAREST_H
#define __MATH_BOUNDED_NEAREST_H

#include <cmath>
#include <QtGlobal>

namespace math
{
  template<typename U, typename T>
  U bounded_nearest (const T& value)
  {
    return qBound ( T (std::numeric_limits<U>::min())
                  , T (value > T() ? ceil (value) : floor (value))
                  , T (std::numeric_limits<U>::max())
                  );
  }
}

#endif
