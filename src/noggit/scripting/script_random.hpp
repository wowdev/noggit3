// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>

#include <random>
#include <chrono>
#include <memory>
namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    class random: public script_object
    {
    public:
      random(script_context * state, unsigned seed);
      random(script_context * state, std::string const& seed);
      random(script_context * state);
      long integer(random& rand, long low, long high);
      double real(random& rand, double low, double high);
    private:
      std::minstd_rand _state;
    };

    random random_from_seed(script_context * state, std::string const& seed);
    random random_from_time(script_context * state);

    void register_random(script_context * state);
  } // namespace scripting
} // namespace noggit
