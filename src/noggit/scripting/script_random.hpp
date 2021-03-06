// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <random>
#include <chrono>
#include <memory>

namespace noggit
{
  namespace scripting
  {
    struct random
    {
      std::minstd_rand _state;
      random(unsigned seed);
      random(std::string const& seed);
      random();
    };

    int rand_int32(random& rand, int low, int high);
    unsigned rand_uint32(random& rand, unsigned low, unsigned high);
    unsigned long rand_uint64(random& rand, unsigned long low, unsigned long high);
    long rand_int64(random& rand, long low, long high);
    double rand_double(random& rand, double low, double high);
    float rand_float(random& rand, float low, float high);

    random random_from_seed(char const* seed);
    random random_from_time();
  } // namespace scripting
} // namespace noggit
