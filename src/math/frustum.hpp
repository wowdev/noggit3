// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <math/matrix_4x4.hpp>

#include <array>
#include <vector>

namespace math
{
  class frustum
  {
    enum SIDES
    {
      RIGHT,
      LEFT,
      BOTTOM,
      TOP,
      BACK,
      FRONT,
      SIDES_MAX,
    };

    class plane
    {
    public:
      plane() = default;
      plane (vector_4d const& vec)
        : _normal (vec.xyz())
        , _distance (vec.w)
      {
        normalize();
      }

      void normalize()
      {
        const float recip (1.0f / _normal.length());
        _normal *= recip;
        _distance *= recip;
      }

      const float& distance() const
      {
        return _distance;
      }

      const vector_3d& normal() const
      {
        return _normal;
      }

    private:
      vector_3d _normal;
      float _distance;
    };
    std::array<plane, SIDES_MAX> _planes;

  public:
    frustum (matrix_4x4 const& matrix);

    bool contains (const vector_3d& point) const;
    bool intersects (const std::vector<vector_3d>& intersect_points) const;
    bool intersects ( const vector_3d& v1
                    , const vector_3d& v2
                    ) const;
    bool intersectsSphere ( const vector_3d& position
                          , const float& radius
                          ) const;
  };
}
