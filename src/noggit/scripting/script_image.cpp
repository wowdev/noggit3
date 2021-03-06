// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <daScript/daScript.h>

#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_heap.hpp>

#include <lodepng.h>

namespace noggit
{
  namespace scripting
  {
    static void img_resize(script_image& img, int width, int height, das::Context* ctx)
    {
      if(width<=0||height<=0)
      {
        throw script_exception(
          "img_resize",
          std::string("tried to resize to invalid image size: x=")
          + std::to_string(width)
          + std::string(" y=")
          + std::to_string(height)
        );
      }
      // m is for more fun
      img._size = width*height*4;
      img._width = width;
      img._height = height;
      img._image = script_calloc(img._size, ctx);
    }

    script_image load_png(char const* path, das::Context* ctx)
    {
      script_image img;
      if(path==nullptr)
      {
          throw script_exception("load_png","empty png path");
      }
      // annoying, but lodepng only takes a vector
      std::vector<unsigned char> vec;
      unsigned error = lodepng::decode(vec, img._width, img._height, path);
      if (error)
      {
        throw script_exception(
          "img_load_png",
          "failed to load png image with error code:" 
          + std::to_string (error));
      }
      img_resize(img, img._width, img._height, ctx);
      memcpy(img.get_image(), vec.data(), vec.size());
      return img;
    }

    script_image create_image(int width, int height, das::Context * ctx)
    {
      script_image img;
      img_resize(img,width,height, ctx);
      return img;
    }

    int img_width(script_image const& img)
    {
      return img._width;
    }

    int img_height(script_image const& img)
    {
      return img._height;
    }

    int img_get_index(script_image const& img, int x, int y)
    {
      int index = ((x + y * img._width) * 4);
      if(index<0||index>=img._size)
      {
        throw script_exception(
          "img_get_index",
          "image coordinates out of bounds: x="
            + std::to_string(x)
            + " y="
            + std::to_string(y)
            + " width="
            + std::to_string(img._width)
            + " height="
            + std::to_string(img._height));
      }
      return index;
    }

    unsigned img_get_pixel(script_image const& img, int x, int y)
    {
      unsigned index = img_get_index(img, x, y);
      return img.get_image()[index] << 24 
        | img.get_image()[index + 1] << 16 
        | img.get_image()[index + 2] << 8 
        | img.get_image()[index + 3];
    }

    void img_set_pixel(script_image& img, int x, int y, unsigned value)
    {
      unsigned index = img_get_index(img, x, y);
      img.get_image()[index] = (value << 24);
      img.get_image()[index + 1] = (value << 16) & 0xff;
      img.get_image()[index + 2] = (value << 8) & 0xff;
      img.get_image()[index + 3] = (value) & 0xff;
    }

    void img_save(script_image& img, char const* filename)
    {
      unsigned error = lodepng::encode(filename, img.get_image(), img._width, img._height);
      if (error)
      {
        throw script_exception(
          "img_save",
          "failed to save image with error "
          + std::to_string (error));
      }
    }

    float img_gradient_scale(script_image const& img, float rel)
    {
      if(rel<0||rel>=1)
      {
        throw script_exception(
          "img_gradient_scale",
          "relative image coordinate out of bounds: "
            + std::to_string(rel)
            + " (should be >= 0 and < 1)");
      }
      int x = std::floor(rel * float(img._width));
      // read red channel, but it shouldn't matter.
      return float(img.get_image()[x * 4]) / 255.0;
    }
  } // namespace scripting
} // namespace noggit
