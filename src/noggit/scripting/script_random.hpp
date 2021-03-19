// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <random>
#include <chrono>
#include <memory>

namespace sol {
  class state;
}

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class lua_state;
    struct random
    {
      std::minstd_rand _state;
      random(unsigned seed);
      random(std::string const& seed);
      random();
      long integer(random& rand, long low, long high);
      double real(random& rand, double low, double high);
    };

    random random_from_seed(std::string const& seed);
    random random_from_time();

    void register_random(lua_state * state);
  } // namespace scripting
} // namespace noggit
