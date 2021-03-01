// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_heap.hpp>

namespace noggit
{
  namespace scripting
  {
    script_noise_map make_noisemap()
    {
      return script_noise_map();
    }

    float noise_get_index(script_noise_map& noise, int x, int y)
    {
      unsigned index = x + y * noise._width;
      return noise._noise[index];
    }

    float noise_get_global(script_noise_map& noise, math::vector_3d& pos)
    {
      return noise_get_index(noise, std::round(pos.x) - noise._start_x, std::round(pos.z) - noise._start_y);
    }

    bool noise_is_highest_global(script_noise_map& noise, math::vector_3d& pos, int check_radius)
    {
      int x = std::round(pos.x) - noise._start_x;
      int z = std::round(pos.z) - noise._start_y;

      float own = noise_get_index(noise, x, z);

      for (int xc = x - check_radius; xc < x + check_radius; ++xc)
      {
        for (int zc = z - check_radius; zc < z + check_radius; ++zc)
        {
          if (xc == x && zc == z)
          {
            continue;
          }

          if (noise_get_index(noise, xc, zc) > own)
          {
            return false;
          }
        }
      }
      return true;
    }

    void noise_set(script_noise_map& noise, int x, int y, float value)
    {
      unsigned index = x + y * noise._width;
      noise._noise[index] = value;
    }

    void script_noise_map::resize(unsigned width, unsigned height, unsigned start_x, unsigned start_y)
    {
      _width = width;
      _height = height;
      _start_x = start_x;
      _start_y = start_y;
      if (_noise != nullptr)
      {
        script_free(_noise);
      }
      _noise = (float*)script_malloc(width * height * sizeof(float));
    }

    unsigned noise_start_x(script_noise_map& noise) { return noise._start_x; }
    unsigned noise_start_y(script_noise_map& noise) { return noise._start_y; }
    unsigned noise_width(script_noise_map& noise) { return noise._width; }
    unsigned noise_height(script_noise_map& noise) { return noise._height; }

    script_noise_generator::script_noise_generator(script_noise_wrapper* wrapper)
      : _wrapper(wrapper) {}

    void noise_fill(script_noise_generator& thiz, script_noise_map& map, char const* seed, int x_start, int y_start, unsigned x_size, unsigned y_size, float frequency)
    {
      map.resize(x_size, y_size, x_start, y_start);
      map._start_x = x_start;
      map._start_y = y_start;
      map._width = x_size;
      map._height = y_size;
      thiz._wrapper->_generator->GenUniformGrid2D(map._noise, x_start, y_start, x_size, y_size, frequency, std::hash<std::string>()(std::string(seed)));
    }

    void noise_fill_selection(script_noise_generator& thiz, script_noise_map& map, script_selection& selection, char const* seed, float frequency, int padding)
    {
      auto x_start = std::floor(selection._min.x) - (padding + 1);
      auto z_start = std::floor(selection._min.z) - (padding + 1);

      auto x_size = std::ceil(selection._max.x - selection._min.x) + (padding + 1) * 2;
      auto z_size = std::ceil(selection._max.z - selection._min.z) + (padding + 1) * 2;

      noise_fill(thiz, map, seed, x_start, z_start, x_size, z_size, frequency);
    }

    static script_noise_wrapper* wrap(FastNoise::SmartNode<> generator)
    {
      script_noise_wrapper* wrap =
        (script_noise_wrapper*)script_calloc(sizeof(script_noise_wrapper));
      wrap->_generator = generator;
      return wrap;
    }

    script_noise_generator make_noisegen_simplex()
    {
      return script_noise_generator(wrap(FastNoise::New<FastNoise::Simplex>()));
    }
    script_noise_generator make_noisegen_perlin()
    {
      return script_noise_generator(wrap(FastNoise::New<FastNoise::Perlin>()));
    }
    script_noise_generator make_noisegen_value()
    {
      return script_noise_generator(wrap(FastNoise::New<FastNoise::Value>()));
    }
    script_noise_generator make_noisegen_fractal()
    {
      return script_noise_generator(wrap(FastNoise::New<FastNoise::FractalFBm>()));
    }
    script_noise_generator make_noisegen_cellular()
    {
      return script_noise_generator(wrap(FastNoise::New<FastNoise::CellularValue>()));
    }
    script_noise_generator make_noisegen_white()
    {
      return script_noise_generator(wrap(FastNoise::New<FastNoise::White>()));
    }
    script_noise_generator make_noisegen_custom(char const* encodedNodeTree)
    {
      return script_noise_generator(wrap(FastNoise::NewFromEncodedNodeTree(encodedNodeTree)));
    }
  } // namespace scripting
} // namespace noggit
