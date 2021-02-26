// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace noggit
{
  namespace scripting
  {
    struct script_image
    {
      script_image() {}
      unsigned char *_image = nullptr;
      unsigned _width = 0;
      unsigned _height = 0;
    };

    int img_get_index(script_image &img, int x, int y);
    unsigned img_get_pixel(script_image &img, int x, int y);

    float img_gradient_scale(script_image &img, float rel);

    void img_set_pixel(script_image &img, int x, int y, unsigned value);
    void img_save(script_image &img, const char *filename);
    int img_width(script_image &img);
    int img_height(script_image &img);
    void img_resize(script_image &img, int width, int height);
    void img_load_png(script_image &img, const char *path);

    script_image create_image();
  } // namespace scripting
} // namespace noggit
