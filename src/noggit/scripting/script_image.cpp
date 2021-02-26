// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_heap.hpp>

#include <lodepng.h>

namespace noggit
{
  namespace scripting
  {
    void img_load_png(script_image &img, const char *path)
    {
      // annoying, but lodepng only takes a vector
      std::vector<unsigned char> vec;
      unsigned error = lodepng::decode(vec, img._width, img._height, path);
      if (error)
      {
        throw script_exception("Failed to load png image with error code:" + error);
      }
      img_resize(img, img._width, img._height);
      memcpy(img._image, vec.data(), vec.size());
    }

    void img_resize(script_image &img, int width, int height)
    {
      if (img._image != nullptr)
      {
        script_free(img._image);
      }
      // m is for more fun
      img._image = (unsigned char *)script_malloc(width * height * 4);
      img._width = width;
      img._height = height;
    }

    int img_width(script_image &img)
    {
      return img._width;
    }

    int img_height(script_image &img)
    {
      return img._height;
    }

    int img_get_index(script_image &img, int x, int y)
    {
      return ((x + y * img._width) * 4);
    }

    unsigned img_get_pixel(script_image &img, int x, int y)
    {
      unsigned index = img_get_index(img, x, y);
      return img._image[index] << 24 | img._image[index + 1] << 16 | img._image[index + 2] << 8 | img._image[index + 3];
    }

    script_image create_image()
    {
      return script_image();
    }

    void img_set_pixel(script_image &img, int x, int y, unsigned value)
    {
      unsigned index = img_get_index(img, x, y);
      img._image[index] = (value << 24);
      img._image[index + 1] = (value << 16) & 0xff;
      img._image[index + 2] = (value << 8) & 0xff;
      img._image[index + 3] = (value)&0xff;
    }

    void img_save(script_image &img, const char *filename)
    {
      unsigned error = lodepng::encode(filename, img._image, img._width, img._height);
      if (error)
      {
        get_cur_tool()->addLog("[error]: failed to save image with error " + error);
      }
    }

    float img_gradient_scale(script_image &img, float rel)
    {
      int x = std::floor(rel * float(img._width));
      // read red channel, but it shouldn't matter.
      return float(img._image[x * 4]) / 255.0;
    }
  } // namespace scripting
} // namespace noggit