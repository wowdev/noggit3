// This file is part of Noggit3, licensed under GNU General Public License (version 3).

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
