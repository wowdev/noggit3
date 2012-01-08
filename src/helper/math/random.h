#ifndef __HELPER_MATH_RANDOM_H
#define __HELPER_MATH_RANDOM_H

namespace helper
{
  namespace math
  {
    namespace random
    {
      template<typename T>
      T floating_point (const T& min, const T& max)
      {
        return min + (max - min) * qrand() / T (RAND_MAX);
      }

      template<typename T>
      T integer (const T& min, const T& max)
      {
        return min + qrand() % (max - min + 1);
      }
    }
  }
}

#endif
