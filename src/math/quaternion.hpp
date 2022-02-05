// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/interpolation.hpp>
#include <math/vector_4d.hpp>

#include <cstdint>

namespace math
{
  struct vector_3d;

  struct quaternion : public vector_4d
  {
  public:
    quaternion()
      : quaternion (0.f, 0.f, 0.f, 1.0f)
    {}

    quaternion ( const float& x_
               , const float& y_
               , const float& z_
               , const float& w_
               )
      : vector_4d(x_, y_, z_, w_)
    { }

    explicit quaternion (const vector_4d& v)
      : vector_4d(v)
    { }

    quaternion (const vector_3d& v, const float w_)
      : vector_4d(v, w_)
    { }

    // heading = rotation around y
    // attitude = rotation around z
    // bank = rotation around x
    quaternion(math::radians bank, math::radians heading, math::radians attitude) : vector_4d()
    {
      heading._ *= 0.5;
      attitude._ *= 0.5;
      bank._ *= 0.5;
      double c1 = cos(heading );
      double s1 = sin(heading );
      double c2 = cos(attitude);
      double s2 = sin(attitude );
      double c3 = cos(bank );
      double s3 = sin(bank );
      double c1c2 = c1 * c2;
      double s1s2 = s1 * s2;
      w = static_cast<float>(c1c2 * c3 - s1s2 * s3);
      x = static_cast<float>(c1c2 * s3 + s1s2 * c3);
      y = static_cast<float>(s1 * c2 * c3 + c1 * s2 * s3);
      z = static_cast<float>(c1 * s2 * c3 - s1 * c2 * s3);
    }

    quaternion operator% (const quaternion& q2) const
    {
      float newx = x * q2.w + y * q2.z - z * q2.y + w * q2.x;
      float newy = -x * q2.z + y * q2.w + z * q2.x + w * q2.y;
      float newz = x * q2.y - y * q2.x + z * q2.w + w * q2.z;
      float neww = -x * q2.x - y * q2.y - z * q2.z + w * q2.w;
      return quaternion(newx, newy, newz, neww);
    }

    degrees::vec3 ToEulerAngles() const
    {
      math::degrees::vec3 retVal;

      float sqw = w * w;
      float sqx = x * x;
      float sqy = y * y;
      float sqz = z * z;
      float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
      float test = x * y + z * w;
      if (test > 0.499f * unit) // singularity at north pole
      {
        retVal.y = -math::degrees(math::radians(2.0f * std::atan2(x, w)));
        retVal.x = math::degrees(math::radians(math::constants::pi / 2.f));
        retVal.z = 0_deg;
      }
      else if (test < -0.499f * unit) // singularity at south pole
      {
        retVal.y = -math::degrees(math::radians(-2.0f * std::atan2(x, w)));
        retVal.x = -math::degrees(math::radians(math::constants::pi / 2.f));
        retVal.z = 0_deg;
      }
      else
      {
        retVal.y = -math::degrees(math::radians(std::atan2(2.f * y * w - 2.f * x * z, sqx - sqy - sqz + sqw)));
        retVal.x = math::degrees(math::radians(std::asin(2.f * test / unit)));
        retVal.z = math::degrees(math::radians(std::atan2(2.f * x * w - 2.f * y * z, -sqx + sqy - sqz + sqw)));
      }

      return retVal;
    }
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
