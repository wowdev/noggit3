// random.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __MATH_RANDOM_H
#define __MATH_RANDOM_H

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

#endif
