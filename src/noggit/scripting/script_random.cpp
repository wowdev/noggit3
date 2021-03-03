// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_exception.hpp>

namespace noggit
{
  namespace scripting
  {
    script_random::script_random(unsigned seed)
      : _state(seed)
    {
    }

    script_random::script_random(std::string const& seed)
      : _state(std::hash<std::string>()(seed))
    {
    }

    script_random::script_random()
      : _state(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
    }

    int rand_int32(script_random& rand, int low, int high)
    {
      if(low>=high) throw script_exception("random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      return std::uniform_int_distribution<int>(low,high-1)(rand._state);
    }

    unsigned rand_uint32(script_random& rand, unsigned low, unsigned high)
    {
      if(low>=high) throw script_exception("random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      return std::uniform_int_distribution<unsigned>(low,high-1)(rand._state);
    }

    unsigned long rand_uint64(script_random& rand, unsigned long low, unsigned long high)
    {
      if(low>=high) throw script_exception("random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      return std::uniform_int_distribution<unsigned long>(low,high-1)(rand._state);
    }

    long rand_int64(script_random& rand, long low, long high)
    {
      if(low>=high) throw script_exception("random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      return std::uniform_int_distribution<long>(low,high-1)(rand._state);
    }

    double rand_double(script_random& rand, double low, double high)
    {
      if(low>=high) throw script_exception("random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      double val;
      do {
        val = std::uniform_real_distribution<double>(low,high)(rand._state);
      } while(val == high);
      return val;
    }

    float rand_float(script_random& rand, float low, float high)
    {
      if(low>=high) throw script_exception("random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      float val;
      do {
        val = std::uniform_real_distribution<float>(low,high)(rand._state);
      } while(val == high);
      return val;
    }

    script_random random_from_seed(char const* seed)
    {
      return script_random(std::string(seed));
    }

    script_random random_from_time()
    {
      return script_random();
    }
  } // namespace scripting
} // namespace noggit