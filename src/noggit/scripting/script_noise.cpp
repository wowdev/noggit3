// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_heap.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <das/Context.fwd.hpp>

#include <boost/algorithm/string.hpp>

namespace noggit
{
  namespace scripting
  {
    namespace
    {
      float noise_get_index(std::string const& caller, noisemap const& noise, int x, int y)
      {
        unsigned index = x + y * noise._width;
        if(index<0||index>=noise._size)
        {
          throw script_exception(
            caller,
            std::string("noise coordinates out of bounds: x=")
              + std::to_string(x)
              + std::string(" y=")
              + std::to_string(y)
              + std::string(" width=")
              + std::to_string(noise._width)
              + std::string(" height=")
              + std::to_string(noise._height));
        }
        return noise.get_map()[index];
      }
    }

    float noise_get(noisemap& noise, math::vector_3d& pos)
    {
      return noise_get_index("noise_get",noise, std::round(pos.x) - noise._start_x, std::round(pos.z) - noise._start_y);
    }

    bool noise_is_highest(noisemap& noise, math::vector_3d& pos, int check_radius)
    {
      int x = std::round(pos.x) - noise._start_x;
      int z = std::round(pos.z) - noise._start_y;

      float own = noise_get_index("noise_is_highest",noise, x, z);

      for (int xc = x - check_radius; xc < x + check_radius; ++xc)
      {
        for (int zc = z - check_radius; zc < z + check_radius; ++zc)
        {
          if (xc == x && zc == z)
          {
            continue;
          }

          if (noise_get_index("noise_is_highest",noise, xc, zc) > own)
          {
            return false;
          }
        }
      }
      return true;
    }

    void noise_set(noisemap& noise, int x, int y, float value)
    {
      unsigned index = x + y * noise._width;
      if(index<0||index>=noise._size)
      {
        throw script_exception(
          "noise_set",
          std::string("noisemap coordinates out of bounds: x=")
            + std::to_string(x)
            + std::string(" y=")
            + std::to_string(y)
            + std::string(" width=")
            + std::to_string(noise._width)
            + std::string(" height=")
            + std::to_string(noise._height));
      }
      noise.get_map()[index] = value;
    }

    noisemap::noisemap(
        unsigned start_x
      , unsigned start_y
      , unsigned width
      , unsigned height
      , float frequency
      , const char* algorithm
      , const char* seed
      , das::Context * ctx)
    {
      if(algorithm==nullptr)
      {
          throw script_exception(
              "make_script_noise"
              , std::string("invalid noise algorithm (empty string)")
          );
      }
      if (seed == nullptr)
      {
          seed = "";
      }

      if(width<=0||height<=0)
      {
        throw script_exception(
          "make_script_noise",
          std::string("invalid noise map size:")
          + " width="
          + std::to_string(width)
          + " height="
          + std::to_string(height)
        );
      }
      _width = width;
      _height = height;
      _start_x = start_x;
      _start_y = start_y;
      _size = width*height;
      _noise = script_calloc(width * height * sizeof(float), ctx);

      auto upper = boost::algorithm::to_upper_copy<std::string>(algorithm);

      FastNoise::SmartNode<> generator = nullptr;
      if(upper=="SIMPLEX")
      {
        generator = FastNoise::New<FastNoise::Simplex>();
      }
      else if(upper=="PERLIN")
      {
        generator = FastNoise::New<FastNoise::Perlin>();
      }
      else if(upper=="VALUE")
      {
        generator = FastNoise::New<FastNoise::Value>();
      }
      else if(upper=="FRACTAL")
      {
        generator = FastNoise::New<FastNoise::FractalFBm>();
      }
      else if(upper=="CELLULAR")
      {
        generator = FastNoise::New<FastNoise::CellularValue>();
      }
      else if(upper=="WHITE")
      {
        generator = FastNoise::New<FastNoise::White>();
      }
      else
      {
        generator = FastNoise::NewFromEncodedNodeTree(algorithm);
      }

      generator->GenUniformGrid2D(
        get_map(),
        start_x,
        start_y,
        width,
        height,
        frequency,
        std::hash<std::string>()(std::string(seed))
      );
    }

    math::vector_3d noise_start(noisemap const& noise)
    {
      return math::vector_3d(noise._start_x,0,noise._start_y);
    }

    unsigned noise_width(noisemap& noise)
    {
      return noise._width;
    }
    unsigned noise_height(noisemap& noise)
    {
      return noise._height;
    }

    noisemap make_noise_size(
        int x_start
      , int y_start
      , int x_size
      , int y_size
      , float frequency
      , const char* algorithm
      , const char* seed
      , das::Context* ctx)
    {
      return noisemap(
        x_start
        , y_start
        , x_size
        , y_size
        , frequency
        , algorithm
        , seed
        , ctx);
    }

    noisemap make_noise_selection(
      selection const& selection
      , float frequency
      , int padding
      , const char* algorithm
      , const char* seed
      , das::Context* ctx)
    {
      auto x_start = std::floor(selection._min.x) - (padding + 1);
      auto z_start = std::floor(selection._min.z) - (padding + 1);
      auto x_size = std::ceil(selection._max.x - selection._min.x) + (padding + 1) * 2;
      auto z_size = std::ceil(selection._max.z - selection._min.z) + (padding + 1) * 2;
      return noisemap(x_start,z_start,x_size,z_size,frequency,algorithm,seed, ctx);
    }
  } // namespace scripting
} // namespace noggit
