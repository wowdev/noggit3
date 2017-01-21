// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/shader.hpp>

namespace math
{
  struct vector_3d;
  struct vector_4d;
}

namespace opengl
{
  namespace primitives
  {
    class wire_box
    {
    public:
      wire_box ( math::vector_3d const& min_point
               , math::vector_3d const& max_point
               );

      void draw (math::vector_4d const& color, float line_width) const;

    private:
      union
      {
        GLuint _buffers[2];
        struct
        {
          GLuint _positions;
          GLuint _indices;
        };
      };
      opengl::program _program;
    };
  }
}
