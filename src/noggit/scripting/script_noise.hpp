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
          , std::string const& algorithm 
          , std::string const& seed);
      noisemap() = default;

      float get(math::vector_3d &pos);
      bool is_highest(math::vector_3d &pos, int check_radius);
      void set(int x, int y, float value);
      math::vector_3d start();
      unsigned width();
      unsigned height();
    
    private:
      std::vector<float> _noise;
      float get_index(std::string const& caller, int x, int y);
      float *get_map() { return _noise.data(); };
      unsigned _width = 0;
      unsigned _height = 0;
      unsigned _start_x = 0;
      unsigned _start_y = 0;
      unsigned _size = 0;
    };

    std::shared_ptr<noisemap> make_noise(
        int start_x
      , int start_y
      , int width
      , int height
      , float frequency
      , std::string const& algorithm
      , std::string const& seed);

    void register_noise(script_context * state);
  } // namespace scripting
} // namespace noggit
