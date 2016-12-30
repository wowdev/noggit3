// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cmath>

namespace math
{
  namespace interpolation
  {
    template<typename T>
    static T linear (const float& percentage, const T& start, const T& end)
    {
      return T (start * (1.0f - percentage) + end * percentage);
    }

    template<typename T>
    static T slerp (const float& percentage, const T& start, const T& end)
    {
      const float dot (start * end);

      if (std::abs (dot) > 0.9995f)
      {
        //! \note Don't call linear here, as this will recurse with quaternions.
        return T (start * (1.0f - percentage) + end * percentage);
      }

      const float a (acosf (dot) * percentage);

      return T (start * cosf (a) + T (end - start * dot).normalize() * sinf (a));
    }

    template<typename T>
    static T hermite ( const float& percentage
                     , const T& start
                     , const T& end
                     , const T& in
                     , const T& out
                     )
    {
      const float percentage_2 (percentage * percentage);
      const float percentage_3 (percentage_2 * percentage);
      const float _2_percentage_3 (2.0f * percentage_3);
      const float _3_percentage_2 (3.0f * percentage_2);

      const float h1 (_2_percentage_3 - _3_percentage_2 + 1.0f);
      const float h2 (_3_percentage_2 - _2_percentage_3);
      const float h3 (percentage_3 - 2.0f * percentage_2 + percentage);
      const float h4 (percentage_3 - percentage_2);

      return T (start * h1 + end * h2 + in * h3 + out * h4);
    }
  }
}
