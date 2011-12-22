#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include "Vec3D.h"
#include "Quaternion.h"

static double PI = 3.14159265358;

class Matrix {
public:
  float m[4][4];

  Matrix()
  {
  }

  Matrix(const Matrix& p)
  {
    memcpy( &m[0][0], &p.m[0][0], sizeof( m ) );
  }

  Matrix& operator= (const Matrix& p)
  {
    memcpy( &m[0][0], &p.m[0][0], sizeof( m ) );
    return *this;
  }

  inline void zero()
  {
    memset( &m[0][0], 0, sizeof( m ) );
  }

  inline void unit()
  {
    zero();
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
  }

  inline void translation(const Vec3D& tr)
  {
    /*
      100#
      010#
      001#
      0001
    */
    unit();
    m[0][3]=tr.x;
    m[1][3]=tr.y;
    m[2][3]=tr.z;
  }

  static inline const Matrix newTranslation(const Vec3D& tr)
  {
    Matrix t;
    t.translation(tr);
    return t;
  }

  inline void scale(const Vec3D& sc)
  {
    /*
      #000
      0#00
      00#0
      0001
    */
    zero();
    m[0][0]=sc.x;
    m[1][1]=sc.y;
    m[2][2]=sc.z;
    m[3][3]=1.0f;
  }

  static inline const Matrix newScale(const Vec3D& sc)
  {
    Matrix t;
    t.scale(sc);
    return t;
  }

  inline void quaternionRotate(const Quaternion& q)
  {
    /*
      ###0
      ###0
      ###0
      0001
    */
    m[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
    m[0][1] = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
    m[0][2] = 2.0f * q.x * q.z - 2.0f * q.w * q.y;
    m[1][0] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
    m[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
    m[1][2] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;
    m[2][0] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
    m[2][1] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
    m[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
    m[0][3] = m[1][3] = m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.0f;
    m[3][3] = 1.0f;
  }

  inline void rotate(const Vec3D& r)
  {
    /*
      ###0
      ###0
      ###0
      0001
    */
    Matrix rot;
    float cosV,sinV;

    //Rotate around the Y axis by B-90
    rot.unit();
    cosV=cosf((r.y-90.0f)*PI/180.0f);
    sinV=sinf((r.y-90.0f)*PI/180.0f);
    rot.unit();
    rot.m[0][0]=cosV;
    rot.m[0][2]=sinV;
    rot.m[2][0]=-sinV;
    rot.m[2][2]=cosV;
    operator*=(rot);

    //Rotate around the Z axis by -A
    rot.unit();
    cosV=cosf(-r.x*PI/180.0f);
    sinV=sinf(-r.x*PI/180.0f);
    rot.unit();
    rot.m[0][0]=cosV;
    rot.m[0][1]=sinV;
    rot.m[1][0]=-sinV;
    rot.m[1][1]=cosV;
    operator*=(rot);

    //Rotate around the X axis by C
    cosV=cosf(r.z*PI/180.0f);
    sinV=sinf(r.z*PI/180.0f);
    rot.unit();
    rot.m[1][1]=cosV;
    rot.m[1][2]=sinV;
    rot.m[2][1]=-sinV;
    rot.m[2][2]=cosV;
    operator*=(rot);
  }

  static inline const Matrix newQuatRotate( const Quaternion& qr )
  {
    Matrix t;
    t.quaternionRotate( qr );
    return t;
  }

  static inline const Matrix newRotate( const Vec3D& r )
  {
    Matrix t;
    t.rotate( r );
    return t;
  }

  inline Vec3D operator*( const Vec3D& v ) const
  {
    const float x = v.x, y = v.y, z = v.z;
    const float x_ = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
    const float y_ = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
    const float z_ = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
    return Vec3D(x_, y_, z_);
  }

  inline Matrix operator*( const Matrix& p ) const
  {
    Matrix o;
    const float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
    const float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
    const float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
    const float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];
    const float pm00 = p.m[0][0], pm01 = p.m[0][1], pm02 = p.m[0][2], pm03 = p.m[0][3];
    const float pm10 = p.m[1][0], pm11 = p.m[1][1], pm12 = p.m[1][2], pm13 = p.m[1][3];
    const float pm20 = p.m[2][0], pm21 = p.m[2][1], pm22 = p.m[2][2], pm23 = p.m[2][3];
    const float pm30 = p.m[3][0], pm31 = p.m[3][1], pm32 = p.m[3][2], pm33 = p.m[3][3];

    o.m[0][0] = m00 * pm00 + m01 * pm10 + m02 * pm20 + m03 * pm30;
    o.m[0][1] = m00 * pm01 + m01 * pm11 + m02 * pm21 + m03 * pm31;
    o.m[0][2] = m00 * pm02 + m01 * pm12 + m02 * pm22 + m03 * pm32;
    o.m[0][3] = m00 * pm03 + m01 * pm13 + m02 * pm23 + m03 * pm33;
    o.m[1][0] = m10 * pm00 + m11 * pm10 + m12 * pm20 + m13 * pm30;
    o.m[1][1] = m10 * pm01 + m11 * pm11 + m12 * pm21 + m13 * pm31;
    o.m[1][2] = m10 * pm02 + m11 * pm12 + m12 * pm22 + m13 * pm32;
    o.m[1][3] = m10 * pm03 + m11 * pm13 + m12 * pm23 + m13 * pm33;
    o.m[2][0] = m20 * pm00 + m21 * pm10 + m22 * pm20 + m23 * pm30;
    o.m[2][1] = m20 * pm01 + m21 * pm11 + m22 * pm21 + m23 * pm31;
    o.m[2][2] = m20 * pm02 + m21 * pm12 + m22 * pm22 + m23 * pm32;
    o.m[2][3] = m20 * pm03 + m21 * pm13 + m22 * pm23 + m23 * pm33;
    o.m[3][0] = m30 * pm00 + m31 * pm10 + m32 * pm20 + m33 * pm30;
    o.m[3][1] = m30 * pm01 + m31 * pm11 + m32 * pm21 + m33 * pm31;
    o.m[3][2] = m30 * pm02 + m31 * pm12 + m32 * pm22 + m33 * pm32;
    o.m[3][3] = m30 * pm03 + m31 * pm13 + m32 * pm23 + m33 * pm33;
    return o;
  }

  inline float determinant() const
  {
    #define SUB(a,b) (m[2][a]*m[3][b] - m[3][a]*m[2][b])
    return
       m[0][0] * (m[1][1]*SUB(2,3) - m[1][2]*SUB(1,3) + m[1][3]*SUB(1,2))
      -m[0][1] * (m[1][0]*SUB(2,3) - m[1][2]*SUB(0,3) + m[1][3]*SUB(0,2))
      +m[0][2] * (m[1][0]*SUB(1,3) - m[1][1]*SUB(0,3) + m[1][3]*SUB(0,1))
      -m[0][3] * (m[1][0]*SUB(1,2) - m[1][1]*SUB(0,2) + m[1][2]*SUB(0,1));
    #undef SUB
  }

  inline float minor(size_t x, size_t y) const
  {
    float s[3][3];
    for (size_t j=0, v=0; j<4; j++) {
      if (j==y) continue;
      for (size_t i=0, u=0; i<4; ++i) {
        if (i!=x) {
          s[v][u++] = m[j][i];
        }
      }
      v++;
    }
    #define SUB(a,b) (s[1][a]*s[2][b] - s[2][a]*s[1][b])
    return s[0][0] * SUB(1,2) - s[0][1] * SUB(0,2) + s[0][2] * SUB(0,1);
    #undef SUB
  }

  inline const Matrix adjoint() const
  {
    Matrix a;
    for (size_t j=0; j<4; j++) {
      for (size_t i=0; i<4; ++i) {
        a.m[i][j] = (((i+j)&1)?-1.0f:1.0f) * minor(i,j);
      }
    }
    return a;
  }

  inline void invert()
  {
    Matrix adj = adjoint();
    float invdet = 1.0f / determinant();
        for (size_t j=0; j<4; j++) {
          for (size_t i=0; i<4; ++i) {
        m[j][i] = adj.m[j][i] * invdet;
      }
    }
  }

  inline void transpose()
  {
   for (size_t j=1; j<4; j++) {
     for (size_t i=0; i<j; ++i) {
        float f = m[j][i];
        m[j][i] = m[i][j];
        m[i][j] = f;
      }
    }
  }

  inline Matrix& operator*= (const Matrix& p)
  {
    return *this = operator*(p);
  }

  inline operator float*()
  {
    return &m[0][0];
  }
};


#endif

