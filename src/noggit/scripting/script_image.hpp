// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace sol {
  class state;
}

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class image
    {
    public:
      image() = default;
      image(std::string const& path);
      image(int width, int height);
      std::vector<unsigned char> _image;
      int get_index(int x, int y);
      unsigned get_pixel(int x, int y);
      float gradient_scale(float rel);
      void set_pixel(int x, int y, unsigned value);
      void save(std::string const& filename);
      int width();
      int height();
    private:
      void resize(int width, int height);
      unsigned char* get_image() {return _image.data();}
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _size = 0;
    };

    void register_image(sol::state * state, scripting_tool * tool);
  } // namespace scripting
} // namespace noggit
