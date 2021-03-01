// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_random.hpp>

namespace noggit
{
  namespace scripting
  {
    script_random::script_random(unsigned seed)
      : _state(seed)
    {
    }

    script_random::script_random(std::string seed)
      : _state(std::hash<std::string>()(seed))
    {
    }

    script_random::script_random()
      : _state(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
    }

    int rand_int32(script_random& rand, int low, int high)
    {
      return low + int(rand_uint64(rand, 0, std::abs(high - low)));
    }

    unsigned rand_uint32(script_random& rand, unsigned low, unsigned high)
    {
      return rand_uint64(rand, low, high);
    }

    unsigned long rand_uint64(script_random& rand, unsigned long low, unsigned long high)
    {
      auto x = rand._state;
      x ^= x >> 12; // a
      x ^= x << 25; // b
      x ^= x >> 27; // c
      rand._state = x;
      // does the modulo bias cancel out the use of 64 bits here?
      return low + ((x * 0x2545F4914F6CDD1DULL) % (high - low));
    }

    long rand_int64(script_random& rand, long low, long high)
    {
      return low + rand_uint64(rand, 0, std::abs(high - low));
    }

    double rand_double(script_random& rand, double low, double high)
    {
#define RAND_MAX_DOUBLE 18446744073709551615ull
      return low + rand_uint64(rand, 0, RAND_MAX_DOUBLE) / (RAND_MAX_DOUBLE / (high - low));
    }

    float rand_float(script_random& rand, float low, float high)
    {
// TODO: prolly not a good idea
#define RAND_MAX_FLOAT 4294967295
      return low + rand_uint32(rand, 0, RAND_MAX_FLOAT) / (RAND_MAX_FLOAT / (high - low));
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
