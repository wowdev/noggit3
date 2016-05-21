// matrix_4x4.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#pragma once

#include <math/quaternion.h>
#include <stdio.h>

namespace math
{
  class vector_3d;

  class matrix_4x4
  {
  public:
    matrix_4x4()
    { }

    matrix_4x4 (const matrix_4x4& p);

    matrix_4x4 ( float m00, float m01, float m02, float m03
               , float m10, float m11, float m12, float m13
               , float m20, float m21, float m22, float m23
               , float m30, float m31, float m32, float m33
               )
    {
      _m[0][0] = m00; _m[0][1] = m01; _m[0][2] = m02; _m[0][3] = m03;
      _m[1][0] = m10; _m[1][1] = m11; _m[1][2] = m12; _m[1][3] = m13;
      _m[2][0] = m20; _m[2][1] = m21; _m[2][2] = m22; _m[2][3] = m23;
      _m[3][0] = m30; _m[3][1] = m31; _m[3][2] = m32; _m[3][3] = m33;
    }

    matrix_4x4& operator= (const matrix_4x4& p);

    const float& operator() (const size_t& j, const size_t& i) const
    {
      return _m[j][i];
    }
    const float& operator() (const size_t& j, const size_t& i, float value)
    {
      return _m[j][i] = value;
    }

    void zero();
    void unit();

    void translation (const vector_3d& tr);
    void rotate (const quaternion& q);
    void rotate (const vector_3d& r);
    void scale (const vector_3d& sc);

    static inline const matrix_4x4 new_translation_matrix (const vector_3d& tr)
    {
      matrix_4x4 t;
      t.translation(tr);
      return t;
    }

    static inline const matrix_4x4 new_scale_matrix (const vector_3d& sc)
    {
      matrix_4x4 t;
      t.scale (sc);
      return t;
    }

    static inline const matrix_4x4 new_rotation_matrix (const quaternion& qr)
    {
      matrix_4x4 t;
      t.rotate (qr);
      return t;
    }

    static inline const matrix_4x4 new_rotation_matrix (const vector_3d& r)
    {
      matrix_4x4 t;
      t.rotate (r);
      return t;
    }

    vector_3d operator* (const vector_3d& v) const;
    vector_4d operator* (const vector_4d& v) const;
    matrix_4x4 operator* (const matrix_4x4& p) const;

    matrix_4x4& operator* (float);
    matrix_4x4& operator/ (float);

    matrix_4x4 adjoint() const;
    void invert();
    matrix_4x4 inverted() const;
    void transpose();
    matrix_4x4 transposed() const;

    inline matrix_4x4& operator*= (const matrix_4x4& p)
    {
      return *this = operator* (p);
    }

    inline operator float*()
    {
      return _data;
    }
    inline operator const float*() const
    {
      return _data;
    }

    template<size_t i>
    vector_4d column() const
    {
      return ::math::vector_4d (_m[0][i], _m[1][i], _m[2][i], _m[3][i]);
    }

  private:
    enum axis
    {
      x = 0,
      y = 1,
      z = 2,
      num_axis,
    };

    template<axis a>
    inline const matrix_4x4& rotate_axis (const float& radians);

    inline float determinant() const;

    inline float minorSize (size_t x, size_t y) const;
    //minor is keyword on unix

    union
    {
      float _m[4][4];
      float _data[16];
    };
  };
}
