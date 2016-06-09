// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.h>
#include <stdio.h>

namespace math
{
  class vector_3d;

  class matrix_4x4
  {
  public:
    static struct uninitialized_t {} uninitialized;
    matrix_4x4 (uninitialized_t) {}

    static struct zero_t {} zero;
    matrix_4x4 (zero_t)
    {
      memset (_data, 0, sizeof (_data));
    }

    static struct unit_t {} unit;
    matrix_4x4 (unit_t) : matrix_4x4 (zero)
    {
      _m[0][0] = _m[1][1] = _m[2][2] = _m[3][3] = 1.0f;
    }

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

    static struct translation_t {} translation;
    matrix_4x4 (translation_t, vector_3d const& tr)
      : matrix_4x4 ( 1.0f, 0.0f, 0.0f, tr.x()
                   , 0.0f, 1.0f, 0.0f, tr.y()
                   , 0.0f, 0.0f, 1.0f, tr.z()
                   , 0.0f, 0.0f, 0.0f, 1.0f
                   )
    {}

    static struct scale_t {} scale;
    matrix_4x4 (scale_t, vector_3d const& sc)
      : matrix_4x4 ( sc.x(), 0.0f, 0.0f, 0.0f
                   , 0.0f, sc.y(), 0.0f, 0.0f
                   , 0.0f, 0.0f, sc.z(), 0.0f
                   , 0.0f, 0.0f, 0.0f, 1.0f
                   )
    {}

    static struct rotation_t {} rotation;
    matrix_4x4 (rotation_t, quaternion const&);
    matrix_4x4 (rotation_t, vector_3d const& degrees);

    matrix_4x4() = delete;
    matrix_4x4 (matrix_4x4 const&) = default;
    matrix_4x4 (matrix_4x4&&) = default;
    matrix_4x4& operator= (matrix_4x4 const&) = default;
    matrix_4x4& operator= (matrix_4x4&&) = default;
    ~matrix_4x4() = default;

    float operator() (std::size_t const& j, std::size_t const& i) const
    {
      return _m[j][i];
    }
    float operator() (std::size_t const& j, std::size_t const& i, float value)
    {
      return _m[j][i] = value;
    }

    vector_3d operator* (vector_3d const&) const;
    vector_4d operator* (vector_4d const&) const;
    matrix_4x4 operator* (matrix_4x4 const&) const;

    matrix_4x4& operator* (float);
    matrix_4x4& operator/ (float);

    matrix_4x4 adjoint() const;
    matrix_4x4 inverted() const;
    matrix_4x4 transposed() const;

    inline matrix_4x4& operator*= (matrix_4x4 const& p)
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

    template<std::size_t i>
      vector_4d column() const
    {
      return {_m[0][i], _m[1][i], _m[2][i], _m[3][i]};
    }

  private:
    union
    {
      float _m[4][4];
      float _data[16];
    };
  };
}
