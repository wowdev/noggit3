// quaternion.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __MATH_QUATERNION_H
#define __MATH_QUATERNION_H

#include <stdint.h>

#include <math/interpolation.h>
#include <math/vector_4d.h>

namespace math
{
  class vector_3d;

  //! \note Actually, a typedef would be enough.
  class quaternion : public vector_4d
  {
  public:
   quaternion ( const float& x = 0.0f
              , const float& y = 0.0f
              , const float& z = 0.0f
              , const float& w = 1.0f
              )
     : vector_4d (x, y, z, w)
   { }

   explicit quaternion (const vector_4d& v)
     : vector_4d (v)
   { }

   quaternion (const vector_3d& v, const float w)
     : vector_4d (v, w)
   { }
  };

  //! \note "linear" interpolation for quaternions should be slerp by default.
  namespace interpolation
  {
    template<>
    inline quaternion linear ( const float& percentage
                      , const quaternion& start
                      , const quaternion& end
                      )
    {
      return slerp (percentage, start, end);
    }
  }

  //! \note In WoW 2.0+ Blizzard is now storing rotation data in 16bit values instead of 32bit. I don't really understand why as its only a very minor saving in model sizes and adds extra overhead in processing the models. Need this structure to read the data into.
  struct packed_quaternion
  {
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t w;
  };
}

#endif
