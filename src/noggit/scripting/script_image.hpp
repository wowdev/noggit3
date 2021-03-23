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
      int get_index(int x, int y);
      unsigned get_pixel(int x, int y);
      float gradient_scale(float rel);
      void set_pixel(int x, int y, unsigned value);
      void save(std::string const& filename);
      int width();
      int height();
      float get_blue(int x, int y);
      float get_green(int x, int y);
      float get_red(int x, int y);
      float get_alpha(int x, int y);
    private:
      void resize(int width, int height);
      unsigned char* get_image() {return _image.data();}
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _size = 0;
    };

    void register_image(script_context * state);
  } // namespace scripting
} // namespace noggit
