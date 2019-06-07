// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/types.hpp>

namespace opengl
{
  class texture
  {
  public:
    texture();
    ~texture();

    texture (texture const&) = delete;
    texture (texture&&);
    texture& operator= (texture const&) = delete;
    texture& operator= (texture&&);

    virtual void bind();

    static void set_active_texture (size_t num = 0);

  protected:
    typedef GLuint internal_type;

    internal_type _id;
  };
}
