// constants.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#pragma once

namespace math
{
  namespace constants
  {
    inline const float& pi()
    {
      static const float pi (3.14159f);
      return pi;
    }
  }
}
