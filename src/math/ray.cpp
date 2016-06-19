#include <math/ray.hpp>

#include <limits>

namespace math
{
  boost::optional<float> ray::intersect_bounds (vector_3d const& min, vector_3d const& max) const
  {
    float tmin (std::numeric_limits<float>::lowest());
    float tmax (std::numeric_limits<float>::max());

    if (_direction.x() != 0.0f)
    {
      float const tx1 ((min.x() - _origin.x()) / _direction.x());
      float const tx2 ((max.x() - _origin.x()) / _direction.x());

      tmin = std::max (tmin, std::min (tx1, tx2));
      tmax = std::min (tmax, std::max (tx1, tx2));
    }

    if (_direction.y() != 0.0f)
    {
      float const ty1 ((min.y() - _origin.y()) / _direction.y());
      float const ty2 ((max.y() - _origin.y()) / _direction.y());

      tmin = std::max (tmin, std::min (ty1, ty2));
      tmax = std::min (tmax, std::max (ty1, ty2));
    }

    if (_direction.z() != 0.0f)
    {
      float const tz1 ((min.z() - _origin.z()) / _direction.z());
      float const tz2 ((max.z() - _origin.z()) / _direction.z());

      tmin = std::max (tmin, std::min (tz1, tz2));
      tmax = std::min (tmax, std::max (tz1, tz2));
    }

    if (tmax >= tmin)
    {
      return tmin;
    }

    return boost::none;
  }

  boost::optional<float> ray::intersect_triangle (vector_3d const& v0, vector_3d const& v1, vector_3d const& v2) const
  {
    vector_3d const e1 (v1 - v0);
    vector_3d const e2 (v2 - v0);

    vector_3d const P (_direction % e2);

    float const det (e1 * P);

    if (det == 0.0f)
    {
      return boost::none;
    }

    vector_3d const T (_origin - v0);
    float const u ((T * P) / det);

    if (u < 0.0f || u > 1.0f)
    {
      return boost::none;
    }

    vector_3d const Q (T % e1);
    float const v ((_direction * Q) / det);

    if (v < 0.0f || u + v > 1.0f)
    {
      return boost::none;
    }

    float const t ((e2 * Q) / det);

    if (t > std::numeric_limits<float>::min())
    {
      return t;
    }

    return boost::none;
  }
}

