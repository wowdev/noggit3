#include <math/ray.hpp>

namespace math
{
  boost::optional<float> intersect_bounds (ray const& _ray, vector_3d const& _min, vector_3d const& _max)
  {
    double tmin = -INFINITY, tmax = INFINITY;

    if (_ray.direction.x () != 0.0)
    {
      double tx1 = (_min.x () - _ray.origin.x ()) / _ray.direction.x ();
      double tx2 = (_max.x () - _ray.origin.x ()) / _ray.direction.x ();

      tmin = std::max (tmin, std::min (tx1, tx2));
      tmax = std::min (tmax, std::max (tx1, tx2));
    }

    if (_ray.direction.y () != 0.0)
    {
      double ty1 = (_min.y () - _ray.origin.y ()) / _ray.direction.y ();
      double ty2 = (_max.y () - _ray.origin.y ()) / _ray.direction.y ();

      tmin = std::max (tmin, std::min (ty1, ty2));
      tmax = std::min (tmax, std::max (ty1, ty2));
    }

    if (_ray.direction.z () != 0.0)
    {
      double tz1 = (_min.z () - _ray.origin.z ()) / _ray.direction.z ();
      double tz2 = (_max.z () - _ray.origin.z ()) / _ray.direction.z ();

      tmin = std::max (tmin, std::min (tz1, tz2));
      tmax = std::min (tmax, std::max (tz1, tz2));
    }

    if (tmax >= tmin)
      return tmin;

    return boost::none;
  }

  boost::optional<float> intersect_triangle(ray const& _ray, vector_3d const& _v0, vector_3d const& _v1, vector_3d const& _v2)
  {
    static const float EPSILON (0.000001f);

    vector_3d e1 (_v1 - _v0);
    vector_3d e2 (_v2 - _v0);

    vector_3d P (_ray.direction % e2);

    float det (e1 * P);

    if (det > -EPSILON && det < EPSILON)
      return boost::none;

    float inv_det (1.f / det);

    vector_3d T (_ray.origin - _v0);
    float u ((T * P) * inv_det);

    if (u < 0.f || u > 1.f)
      return boost::none;

    vector_3d Q (T % e1);
    float v ((_ray.direction * Q) * inv_det);

    if (v < 0.f || u + v > 1.f)
      return boost::none;

    float t ((e2 * Q) * inv_det);

    if (t > EPSILON)
      return t;

    return boost::none;
  }
}

