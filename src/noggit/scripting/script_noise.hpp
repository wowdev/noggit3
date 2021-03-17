// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_selection.hpp>

#include <math/vector_3d.hpp>

#include <FastNoise/FastNoise.h>
#include <vector>
#include <string>
#include <memory>

namespace noggit
{
  namespace scripting
  {
    class noisemap
    {
    public:
      noisemap(
          unsigned start_x
          , unsigned start_y
          , unsigned width
          , unsigned height
          , float frequency
          , const char *algorithm 
          , const char *seed);
      noisemap() = default;

      float get(math::vector_3d &pos);
      bool is_highest(math::vector_3d &pos, int check_radius);
      void set(int x, int y, float value);
      math::vector_3d start();
      unsigned width();
      unsigned height();
    
    private:
      char *_noise;
      float get_index(std::string const& caller, int x, int y);
      float *get_map() const { return (float *)_noise; };
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _start_x = 0;
      unsigned _start_y = 0;
      unsigned _size = 0;
    };

    noisemap make_noise_size(
        int start_x
      , int start_y
      , int width
      , int height
      , float frequency
      , char const *algorithm
      , char const *seed);

    // TODO: Restore
    /*
    noisemap make_noise_selection(
      selection const& sel
      , float frequency
      , int padding
      , char const *algorithm
      , char const *seed);
    */

    void register_noise(sol::state * state, scripting_tool * tool);
  } // namespace scripting
} // namespace noggit
