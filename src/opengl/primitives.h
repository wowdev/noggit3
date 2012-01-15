#ifndef __OPENGL_PRIMITIVES_H
#define __OPENGL_PRIMITIVES_H

#include <math/vector_3d.h>

namespace math
{
  class vector_4d;
}

namespace opengl
{
  namespace primitives
  {
    class wire_box
    {
    public:
      wire_box ( const ::math::vector_3d& min_point
               , const ::math::vector_3d& max_point
               );

      void draw (const ::math::vector_4d& color, const float& line_width) const;

    private:
      const ::math::vector_3d _min_point;
      const ::math::vector_3d _max_point;
    };
  }
}

#endif
