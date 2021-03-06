// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_heap.hpp>

#include <math/vector_3d.hpp>

#include <FastNoise/FastNoise.h>
#include <vector>
#include <string>
#include <memory>

namespace das
{
  class Context;
}

namespace noggit
{
  namespace scripting
  {
    struct script_noise_map
    {
      script_noise_map(
          unsigned start_x
          , unsigned start_y
          , unsigned width
          , unsigned height 
          , float frequency
          , const char *algorithm 
          , const char *seed
          , das::Context* ctx);
      script_noise_map() = default;
      char *_noise;
      float *get_map() const { return (float *)_noise; };
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _start_x = 0;
      unsigned _start_y = 0;
      unsigned _size = 0;
    };

    float noise_get_index(script_noise_map const &noise, int x, int y);
    float noise_get_global(script_noise_map &noise, math::vector_3d &pos);
    bool noise_is_highest_global(script_noise_map &noise, math::vector_3d &pos, int check_radius);
    void noise_set(script_noise_map &noise, int x, int y, float value);
    unsigned noise_start_x(script_noise_map &noise);
    unsigned noise_start_y(script_noise_map &noise);
    unsigned noise_width(script_noise_map &noise);
    unsigned noise_height(script_noise_map &noise);
    script_noise_map make_noise_size(
        int start_x
      , int start_y
      , int width
      , int height
      , float frequency
      , char const *algorithm
      , char const *seed
      , das::Context* ctx);

    script_noise_map make_noise_selection(
      script_selection const& sel
      , float frequency
      , int padding
      , char const *algorithm
      , char const *seed
      , das::Context* ctx);
  } // namespace scripting
} // namespace noggit
