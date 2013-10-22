// primitives.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <opengl/primitives.h>

#include <math/vector_4d.h>

#include <opengl/types.h>

namespace opengl
{
  namespace primitives
  {
    wire_box::wire_box ( const ::math::vector_3d& min_point
             , const ::math::vector_3d& max_point
             )
      : _min_point (min_point)
      , _max_point (max_point)
    { }

    void wire_box::draw ( const ::math::vector_4d& color
                        , const float& line_width
                        ) const
    {
      glEnable (GL_LINE_SMOOTH);
      glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
      glLineWidth (line_width);

      glColor4fv (color);

      glBegin (GL_LINE_STRIP);
      glVertex3f (_min_point.x(), _max_point.y(), _min_point.z());
      glVertex3f (_min_point.x(), _min_point.y(), _min_point.z());
      glVertex3f (_max_point.x(), _min_point.y(), _min_point.z());
      glVertex3f (_max_point.x(), _min_point.y(), _max_point.z());
      glVertex3f (_max_point.x(), _max_point.y(), _max_point.z());
      glVertex3f (_max_point.x(), _max_point.y(), _min_point.z());
      glVertex3f (_min_point.x(), _max_point.y(), _min_point.z());
      glVertex3f (_min_point.x(), _max_point.y(), _max_point.z());
      glVertex3f (_min_point.x(), _min_point.y(), _max_point.z());
      glVertex3f (_min_point.x(), _min_point.y(), _min_point.z());
      glEnd();

      glBegin (GL_LINES);
      glVertex3f (_min_point.x(), _min_point.y(), _max_point.z());
      glVertex3f (_max_point.x(), _min_point.y(), _max_point.z());
      glEnd();

      glBegin (GL_LINES);
      glVertex3f (_max_point.x(), _max_point.y(), _min_point.z());
      glVertex3f (_max_point.x(), _min_point.y(), _min_point.z());
      glEnd();

      glBegin (GL_LINES);
      glVertex3f (_min_point.x(), _max_point.y(), _max_point.z());
      glVertex3f (_max_point.x(), _max_point.y(), _max_point.z());
      glEnd();

    }
  }
}
