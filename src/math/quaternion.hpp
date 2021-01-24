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
			: quaternion(0.f, 0.f, 0.f, 1.0f)
		{}

		quaternion(const float& x
			, const float& y
			, const float& z
			, const float& w
		)
			: vector_4d(x, y, z, w)
		{ }

		explicit quaternion(const vector_4d& v)
			: vector_4d(v)
		{ }

		quaternion(const vector_3d& v, const float w)
			: vector_4d(v, w)
		{ }
		// heading = rotation around y
		// attitude = rotation around z
		// bank = rotation around x
		quaternion(math::radians bank, math::radians heading, math::radians attitude) : vector_4d()
		{
			/*yawr._ *= 0.5f;
			pitchr._ *= 0.5f;
			rollr._ *= 0.5f;

			// Abbreviations for the various angular functions
			double cy = cos(yawr);
			double sy = sin(yawr);
			double cp = cos(pitchr);
			double sp = sin(pitchr);
			double cr = cos(rollr);
			double sr = sin(rollr);

			w = static_cast<float>(cr * cp * cy + sr * sp * sy);
			x = static_cast<float>(sr * cp * cy - cr * sp * sy);
			y = static_cast<float>(cr * sp * cy + sr * cp * sy);
			z = static_cast<float>(cr * cp * sy - sr * sp * cy);*/
			heading._ *= 0.5;
			attitude._ *= 0.5;
			bank._ *= 0.5;
			// Assuming the angles are in radians.
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

		vector_3d ToEulerAngles() const {
			/* // Old method, prone to gimbal lock!
			double ex, ey, ez;
			// roll (x-axis rotation)
			double sinr_cosp = 2.0 * (w * x + y * z);
			double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
			ex = std::atan2(sinr_cosp, cosr_cosp) * 180.0f / math::constants::pi;

			// pitch (y-axis rotation)
			double sinp = 2.0 * (w * y - z * x);
			if (std::abs(sinp) >= 1)
				ey = std::copysign(math::constants::pi / 2, sinp) * 180.0f / math::constants::pi; // use 90 degrees if out of range
			else
				ey = std::asin(sinp) * 180.0f / math::constants::pi;

			// yaw (z-axis rotation)
			double siny_cosp = 2.0 * (w * z + x * y);
			double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
			 ez = std::atan2(siny_cosp, cosy_cosp) * 180.0f / math::constants::pi;

			return vector_3d(static_cast<float>(ex), static_cast<float>(ey), static_cast<float>(ez));
			*/
			math::vector_3d retVal;

			double sqw = w * w;
			double sqx = x * x;
			double sqy = y * y;
			double sqz = z * z;
			double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
			double test = x * y + z * w;
			if (test > 0.499 * unit) { // singularity at north pole
				retVal.y = -static_cast<float>(2.0f * std::atan2(x, w) * 180.0f / math::constants::pi);
				retVal.x = (math::constants::pi / 2) * 180.0f / math::constants::pi;
				retVal.z = 0;
				return retVal;
			}
			if (test < -0.499 * unit) { // singularity at south pole
				retVal.y = -static_cast<float>(-2.0f * std::atan2(x, w) * 180.0f / math::constants::pi);
				retVal.x = (-math::constants::pi / 2) * 180.0f / math::constants::pi;
				retVal.z = 0;
				return retVal;
			}
			retVal.y = -static_cast<float>(std::atan2(2 * y * w - 2 * x * z, sqx - sqy - sqz + sqw) * 180.0f / math::constants::pi);
			retVal.x = static_cast<float>(std::asin(2 * test / unit) * 180.0f / math::constants::pi);
			retVal.z = static_cast<float>(std::atan2(2 * x * w - 2 * y * z, -sqx + sqy - sqz + sqw) * 180.0f / math::constants::pi);

			return retVal;
		}

	};

	//! \note "linear" interpolation for quaternions should be slerp by default.
	namespace interpolation
	{
		template<>
		inline quaternion linear(const float& percentage
			, const quaternion& start
			, const quaternion& end
		)
		{
			return slerp(percentage, start, end);
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
