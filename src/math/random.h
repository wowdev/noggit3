// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

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
