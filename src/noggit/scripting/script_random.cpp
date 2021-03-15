// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_exception.hpp>

namespace noggit
{
  namespace scripting
  {
    random::random(unsigned seed)
      : _state(seed)
    {
    }

    random::random(std::string const& seed)
      : _state(std::hash<std::string>()(seed))
    {
    }

    random::random()
      : _state(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
    }

    int rand_int32(random& rand, int low, int high)
    {
      if(low>=high) throw script_exception(
        "rand_int32",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      return std::uniform_int_distribution<int>(low,high-1)(rand._state);
    }

    unsigned rand_uint32(random& rand, unsigned low, unsigned high)
    {
      if(low>=high) throw script_exception(
        "rand_uint32",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      return std::uniform_int_distribution<unsigned>(low,high-1)(rand._state);
    }

    unsigned long rand_uint64(random& rand, unsigned long low, unsigned long high)
    {
      if(low>=high)
      {
        throw script_exception(
        "rand_uint64",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      return std::uniform_int_distribution<unsigned long>(low,high-1)(rand._state);
    }

    long rand_int64(random& rand, long low, long high)
    {
      if(low>=high)
      {
        throw script_exception(
        "rand_int64",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      return std::uniform_int_distribution<long>(low,high-1)(rand._state);
    }

    double rand_double(random& rand, double low, double high)
    {
      if(low>=high)
      {
        throw script_exception(
        "rand_double",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      double val;
      do {
        val = std::uniform_real_distribution<double>(low,high)(rand._state);
      } while(val == high);
      return val;
    }

    float rand_float(random& rand, float low, float high)
    {
      if(low>=high)
      {
        throw script_exception(
        "rand_float",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      float val;
      do {
        val = std::uniform_real_distribution<float>(low,high)(rand._state);
      } while(val == high);
      return val;
    }

    random random_from_seed(char const* seed)
    {
      return random(std::string(seed));
    }

    random random_from_time()
    {
      return random();
    }
  } // namespace scripting
} // namespace noggit
