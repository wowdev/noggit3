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
    struct image
    {
      image() = default;
      char* _image;
      unsigned char* get_image() const {return (unsigned char*)_image;}
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _size = 0;
    };

    int img_get_index(image const& img, int x, int y);
    unsigned img_get_pixel(image const& img, int x, int y);

    float img_gradient_scale(image const& img, float rel);

    void img_set_pixel(image& img, int x, int y, unsigned value);
    void img_save(image& img, char const* filename);
    int img_width(image const& img);
    int img_height(image const& img);
    image create_image(int width, int height, das::Context * ctx);
    image load_png(const char* path, das::Context * ctx);
  } // namespace scripting
} // namespace noggit
