// matrix_4x4.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <math/matrix_4x4.h>

#include <cstring> // memcpy, memset
#include <cmath>

#include <math/constants.h>
#include <math/vector_3d.h>
#include <math/quaternion.h>

namespace math
{
  matrix_4x4::matrix_4x4 (const matrix_4x4& p)
  {
    memcpy (&_m[0][0], &p._m[0][0], sizeof(_m));
  }

  matrix_4x4& matrix_4x4::operator= (const matrix_4x4& p)
  {
    memcpy (&_m[0][0], &p._m[0][0], sizeof(_m));
    return *this;
  }

  void matrix_4x4::zero()
  {
    memset (&_m[0][0], 0, sizeof(_m));
  }

  void matrix_4x4::unit()
  {
    zero();
    _m[0][0] = _m[1][1] = _m[2][2] = _m[3][3] = 1.0f;
  }

  void matrix_4x4::translation (const vector_3d& tr)
  {
    unit();
    _m[0][3] = tr.x();
    _m[1][3] = tr.y();
    _m[2][3] = tr.z();
  }

  void matrix_4x4::scale (const vector_3d& sc)
  {
    zero();
    _m[0][0] = sc.x();
    _m[1][1] = sc.y();
    _m[2][2] = sc.z();
    _m[3][3] = 1.0f;
  }

  void matrix_4x4::rotate (const quaternion& q)
  {
    _m[0][0] = 1.0f - 2.0f * q.y() * q.y() - 2.0f * q.z() * q.z();
    _m[0][1] = 2.0f * q.x() * q.y() + 2.0f * q.w() * q.z();
    _m[0][2] = 2.0f * q.x() * q.z() - 2.0f * q.w() * q.y();
    _m[1][0] = 2.0f * q.x() * q.y() - 2.0f * q.w() * q.z();
    _m[1][1] = 1.0f - 2.0f * q.x() * q.x() - 2.0f * q.z() * q.z();
    _m[1][2] = 2.0f * q.y() * q.z() + 2.0f * q.w() * q.x();
    _m[2][0] = 2.0f * q.x() * q.z() + 2.0f * q.w() * q.y();
    _m[2][1] = 2.0f * q.y() * q.z() - 2.0f * q.w() * q.x();
    _m[2][2] = 1.0f - 2.0f * q.x() * q.x() - 2.0f * q.y() * q.y();
    _m[0][3] = _m[1][3] = _m[2][3] = _m[3][0] = _m[3][1] = _m[3][2] = 0.0f;
    _m[3][3] = 1.0f;
  }

  void matrix_4x4::rotate (const vector_3d& r)
  {
    static const float radians_to_angle_constant (constants::pi() / 180.0f);

    matrix_4x4 rot;
    *this *= (rot.rotate_axis<y> ((r.y() - 90.0f) * radians_to_angle_constant));
    *this *= (rot.rotate_axis<z> (-r.x() * radians_to_angle_constant));
    *this *= (rot.rotate_axis<x> (r.z() * radians_to_angle_constant));
  }

  vector_3d matrix_4x4::operator* (const vector_3d& v) const
  {
    const float& x (v.x()), y (v.y()), z (v.z());
    return vector_3d ( _m[0][0] * x + _m[0][1] * y + _m[0][2] * z + _m[0][3]
                     , _m[1][0] * x + _m[1][1] * y + _m[1][2] * z + _m[1][3]
                     , _m[2][0] * x + _m[2][1] * y + _m[2][2] * z + _m[2][3]
                     );
  }

  matrix_4x4 matrix_4x4::operator* (const matrix_4x4& p) const
  {
    const float& m00 (_m[0][0]), m01 (_m[0][1]), m02 (_m[0][2]), m03 (_m[0][3]);
    const float& m10 (_m[1][0]), m11 (_m[1][1]), m12 (_m[1][2]), m13 (_m[1][3]);
    const float& m20 (_m[2][0]), m21 (_m[2][1]), m22 (_m[2][2]), m23 (_m[2][3]);
    const float& m30 (_m[3][0]), m31 (_m[3][1]), m32 (_m[3][2]), m33 (_m[3][3]);
    const float& pm00 (p._m[0][0]), pm01 (p._m[0][1]), pm02 (p._m[0][2]), pm03 (p._m[0][3]);
    const float& pm10 (p._m[1][0]), pm11 (p._m[1][1]), pm12 (p._m[1][2]), pm13 (p._m[1][3]);
    const float& pm20 (p._m[2][0]), pm21 (p._m[2][1]), pm22 (p._m[2][2]), pm23 (p._m[2][3]);
    const float& pm30 (p._m[3][0]), pm31 (p._m[3][1]), pm32 (p._m[3][2]), pm33 (p._m[3][3]);

    matrix_4x4 o;
    o._m[0][0] = m00 * pm00 + m01 * pm10 + m02 * pm20 + m03 * pm30;
    o._m[0][1] = m00 * pm01 + m01 * pm11 + m02 * pm21 + m03 * pm31;
    o._m[0][2] = m00 * pm02 + m01 * pm12 + m02 * pm22 + m03 * pm32;
    o._m[0][3] = m00 * pm03 + m01 * pm13 + m02 * pm23 + m03 * pm33;
    o._m[1][0] = m10 * pm00 + m11 * pm10 + m12 * pm20 + m13 * pm30;
    o._m[1][1] = m10 * pm01 + m11 * pm11 + m12 * pm21 + m13 * pm31;
    o._m[1][2] = m10 * pm02 + m11 * pm12 + m12 * pm22 + m13 * pm32;
    o._m[1][3] = m10 * pm03 + m11 * pm13 + m12 * pm23 + m13 * pm33;
    o._m[2][0] = m20 * pm00 + m21 * pm10 + m22 * pm20 + m23 * pm30;
    o._m[2][1] = m20 * pm01 + m21 * pm11 + m22 * pm21 + m23 * pm31;
    o._m[2][2] = m20 * pm02 + m21 * pm12 + m22 * pm22 + m23 * pm32;
    o._m[2][3] = m20 * pm03 + m21 * pm13 + m22 * pm23 + m23 * pm33;
    o._m[3][0] = m30 * pm00 + m31 * pm10 + m32 * pm20 + m33 * pm30;
    o._m[3][1] = m30 * pm01 + m31 * pm11 + m32 * pm21 + m33 * pm31;
    o._m[3][2] = m30 * pm02 + m31 * pm12 + m32 * pm22 + m33 * pm32;
    o._m[3][3] = m30 * pm03 + m31 * pm13 + m32 * pm23 + m33 * pm33;
    return o;
  }

  const matrix_4x4 matrix_4x4::adjoint() const
  {
    matrix_4x4 a;
    for (size_t j=0; j<4; j++) {
      for (size_t i=0; i<4; ++i) {
        a._m[i][j] = (((i+j)&1)?-1.0f:1.0f) * minorSize(i,j);
      }
    }
    return a;
  }

  void matrix_4x4::invert()
  {
    const matrix_4x4 adj (adjoint());
    const float invdet (1.0f / determinant());
    for (size_t j=0; j<4; j++) {
      for (size_t i=0; i<4; ++i) {
        _m[j][i] = adj._m[j][i] * invdet;
      }
    }
  }
  matrix_4x4 matrix_4x4::inverted() const
  {
    matrix_4x4 result (*this);
    result.invert();
    return result;
  }

  void matrix_4x4::transpose()
  {
   for (size_t j=1; j<4; j++) {
     for (size_t i=0; i<j; ++i) {
        const float f (_m[j][i]);
        _m[j][i] = _m[i][j];
        _m[i][j] = f;
      }
    }
  }

  template<matrix_4x4::axis a>
  inline const matrix_4x4& matrix_4x4::rotate_axis (const float& radians)
  {
    static const int i_indices[num_axis] = {2, 2, 1};
    static const int j_indices[num_axis] = {1, 0, 0};
    const size_t i_index (i_indices[a]);
    const size_t j_index (j_indices[a]);

    const float cosV (cosf (radians));
    const float sinV (sinf (radians));

    unit();
    _m[j_index][j_index] = cosV;
    _m[j_index][i_index] = sinV;
    _m[i_index][j_index] = -sinV;
    _m[i_index][i_index] = cosV;

    return *this;
  }

  inline float matrix_4x4::determinant() const
  {
    #define SUB(a,b) (_m[2][a]*_m[3][b] - _m[3][a]*_m[2][b])
    return
       _m[0][0] * (_m[1][1]*SUB(2,3) - _m[1][2]*SUB(1,3) + _m[1][3]*SUB(1,2))
      -_m[0][1] * (_m[1][0]*SUB(2,3) - _m[1][2]*SUB(0,3) + _m[1][3]*SUB(0,2))
      +_m[0][2] * (_m[1][0]*SUB(1,3) - _m[1][1]*SUB(0,3) + _m[1][3]*SUB(0,1))
      -_m[0][3] * (_m[1][0]*SUB(1,2) - _m[1][1]*SUB(0,2) + _m[1][2]*SUB(0,1));
    #undef SUB
  }

  inline float matrix_4x4::minorSize (size_t x, size_t y) const
  {
    float s[3][3];
    for (size_t j=0, v=0; j<4; j++) {
      if (j==y) continue;
      for (size_t i=0, u=0; i<4; ++i) {
        if (i!=x) {
          s[v][u++] = _m[j][i];
        }
      }
      v++;
    }
    #define SUB(a,b) (s[1][a]*s[2][b] - s[2][a]*s[1][b])
    return s[0][0] * SUB(1,2) - s[0][1] * SUB(0,2) + s[0][2] * SUB(0,1);
    #undef SUB
  }
}
