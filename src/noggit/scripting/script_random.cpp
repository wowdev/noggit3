// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_random.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_context.hpp>
#include <sol/sol.hpp>

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

    long random::integer(random& rand, long low, long high)
    {
      if(low>=high)
      {
        throw script_exception(
        "rand_int64",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      return std::uniform_int_distribution<long>(low,high-1)(rand._state);
    }

    double random::real(random& rand, double low, double high)
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

    random random_from_seed(std::string const& seed)
    {
      return random(std::string(seed));
    }

    random random_from_time()
    {
      return random();
    }

    void register_random(lua_state * state)
    {
      state->new_usertype<random>("random"
        , "integer", &random::integer
        , "real", &random::real
      );
      state->set_function("random_from_seed",random_from_seed);
      state->set_function("random_from_time",random_from_time);
    }
  } // namespace scripting
} // namespace noggit
