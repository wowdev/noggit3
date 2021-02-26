// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <random>
#include <chrono>
#include <memory>

namespace noggit
{
  namespace scripting
  {
    struct script_random
    {
      unsigned long _state;
      script_random(unsigned seed);
      script_random(std::string seed);
      script_random();
    };

    int rand_int32(script_random &rand, int low, int high);
    unsigned rand_uint32(script_random &rand, unsigned low, unsigned high);
    unsigned long rand_uint64(script_random &rand, unsigned long low, unsigned long high);
    long rand_int64(script_random &rand, long low, long high);
    double rand_double(script_random &rand, double low, double high);
    float rand_float(script_random &rand, float low, float high);

    script_random random_from_seed(const char *seed);
    script_random random_from_time();
  } // namespace scripting
} // namespace noggit