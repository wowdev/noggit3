// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace das {
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    struct script_image
    {
      script_image() = default;
      char* _image;
      unsigned char* get_image() const {return (unsigned char*)_image;}
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _size = 0;
    };

    int img_get_index(script_image const& img, int x, int y);
    unsigned img_get_pixel(script_image const& img, int x, int y);

    float img_gradient_scale(script_image const& img, float rel);

    void img_set_pixel(script_image& img, int x, int y, unsigned value);
    void img_save(script_image& img, char const* filename);
    int img_width(script_image const& img);
    int img_height(script_image const& img);
    script_image create_image(int width, int height, das::Context * ctx);
    script_image load_png(const char* path, das::Context * ctx);
  } // namespace scripting
} // namespace noggit
