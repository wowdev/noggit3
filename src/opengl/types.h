// types.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __OPENGL_TYPES_H
#define __OPENGL_TYPES_H

#ifdef __linux__
#include <GL/glew.h>
#else
#include <gl/glew.h>
#endif

namespace opengl
{
  typedef GLuint shader;
  typedef GLuint light;
}

#endif
