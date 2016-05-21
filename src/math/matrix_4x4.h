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
    matrix_4x4 operator* (const matrix_4x4& p) const;

    const matrix_4x4 adjoint() const;
    void invert();
    matrix_4x4 inverted() const;
    void transpose();

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
