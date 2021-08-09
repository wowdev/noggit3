// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>

#include <vector>
#include <string>
#include <memory>

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    class image: public script_object
    {
    public:
      image(script_context * ctx, std::string const& path);
      image(script_context * ctx, int width, int height);
      std::vector<unsigned char> _image;
      int get_index(int x, int y) const;
      unsigned get_pixel(int x, int y) const;
      float gradient_scale(float rel) const;
      void set_pixel(int x, int y, unsigned value);
      void set_pixel_floats(int x, int y, float r, float g, float b, float a /*= 1.0*/);
      void set_pixel_floats_1(int x, int y, float r, float g, float b)
      {
        set_pixel_floats(x,y,r,g,b,1.0);
      }
      void save(std::string const& filename);
      int width() const;
      int height() const;
      float get_blue(int x, int y) const;
      float get_green(int x, int y) const;
      float get_red(int x, int y) const;
      float get_alpha(int x, int y) const;
    private:
      void resize(int width, int height);
      unsigned char const * get_image() const {return _image.data();}
      unsigned char * get_image_w() {return _image.data();}
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _size = 0;
    };

    void register_image(script_context * state);
  } // namespace scripting
} // namespace noggit
