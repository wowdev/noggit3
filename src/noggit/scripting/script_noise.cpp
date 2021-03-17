// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <boost/algorithm/string.hpp>

namespace noggit
{
  namespace scripting
  {
    float noisemap::get_index(std::string const& caller, int x, int y)
    {
      unsigned index = x + y * _width;
      if(index<0||index>=_size)
      {
        throw script_exception(
          caller,
          std::string("noise coordinates out of bounds: x=")
            + std::to_string(x)
            + std::string(" y=")
            + std::to_string(y)
            + std::string(" width=")
            + std::to_string(_width)
            + std::string(" height=")
            + std::to_string(_height));
      }
      return get_map()[index];
    }

    float noisemap::get(math::vector_3d& pos)
    {
      return get_index("noise_get",std::round(pos.x) - _start_x, std::round(pos.z) - _start_y);
    }

    bool noisemap::is_highest(math::vector_3d& pos, int check_radius)
    {
      int x = std::round(pos.x) - _start_x;
      int z = std::round(pos.z) - _start_y;

      float own = get_index("noise_is_highest", x, z);

      for (int xc = x - check_radius; xc < x + check_radius; ++xc)
      {
        for (int zc = z - check_radius; zc < z + check_radius; ++zc)
        {
          if (xc == x && zc == z)
          {
            continue;
          }

          if (get_index("noise_is_highest",xc, zc) > own)
          {
            return false;
          }
        }
      }
      return true;
    }

    void noisemap::set(int x, int y, float value)
    {
      unsigned index = x + y * _width;
      if(index<0||index>=_size)
      {
        throw script_exception(
          "noisemap::set",
          std::string("noisemap coordinates out of bounds: x=")
            + std::to_string(x)
            + std::string(" y=")
            + std::to_string(y)
            + std::string(" width=")
            + std::to_string(_width)
            + std::string(" height=")
            + std::to_string(_height));
      }
      get_map()[index] = value;
    }

    noisemap::noisemap(
        unsigned start_x
      , unsigned start_y
      , unsigned width
      , unsigned height
      , float frequency
      , const char* algorithm
      , const char* seed)
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
      _noise.resize(_size);
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

    math::vector_3d noisemap::start()
    {
      return math::vector_3d(_start_x,0,_start_y);
    }

    unsigned noisemap::width()
    {
      return _width;
    }
    unsigned noisemap::height()
    {
      return _height;
    }

    noisemap make_noise_size(
        int x_start
      , int y_start
      , int x_size
      , int y_size 
      , float frequency 
      , const char* algorithm 
      , const char* seed)
    {
      return noisemap(
        x_start
        , y_start
        , x_size
        , y_size
        , frequency
        , algorithm
        , seed);
    }

    /*
    noisemap make_noise_selection(
      selection const& selection
      , float frequency
      , int padding
      , const char* algorithm
      , const char* seed)
    {
      auto x_start = std::floor(selection._min.x) - (padding + 1);
      auto z_start = std::floor(selection._min.z) - (padding + 1);
      auto x_size = std::ceil(selection._max.x - selection._min.x) + (padding + 1) * 2;
      auto z_size = std::ceil(selection._max.z - selection._min.z) + (padding + 1) * 2;
      return noisemap(x_start,z_start,x_size,z_size,frequency,algorithm,seed);
    }
    */

    void register_noise(sol::state * state, scripting_tool * tool)
    {
      state->new_usertype<noisemap>("noisemap"
        , "get", &noisemap::get
        , "is_highest", &noisemap::is_highest
        , "set", &noisemap::set
        , "start", &noisemap::start
        , "width", &noisemap::width
        , "height", &noisemap::height
      );
      state->set_function("make_noise_size",make_noise_size);
    }
  } // namespace scripting
} // namespace noggit
