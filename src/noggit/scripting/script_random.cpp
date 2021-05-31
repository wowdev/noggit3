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
    random::random(script_context * ctx, unsigned seed)
      : script_object(ctx)
      , _state(seed)
    {
    }

    random::random(script_context * ctx, std::string const& seed)
      : script_object(ctx)
      , _state(std::hash<std::string>()(seed))
    {
    }

    random::random(script_context * ctx)
      : script_object(ctx)
      , _state(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
    }

    long random::integer(long low, long high)
    {
      if (low == high)
      {
        return low;
      }

      if(low>=high)
      {
        throw script_exception(
        "rand_int64",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      return std::uniform_int_distribution<long>(low,high-1)(_state);
    }

    double random::real(double low, double high)
    {
      if (low == high)
      {
        return low;
      }

      if(low>=high)
      {
        throw script_exception(
        "rand_double",
        "random lower bound "+ std::to_string(low) + " >= higher bound " + std::to_string(high));
      }
      double val;
      do {
        val = std::uniform_real_distribution<double>(low,high)(_state);
      } while(val == high);
      return val;
    }

    std::shared_ptr<random> random_from_seed(script_context * ctx, std::string const& seed)
    {
      return std::make_shared<random>(ctx, std::string(seed));
    }

    std::shared_ptr<random> random_from_time(script_context * ctx)
    {
      return std::make_shared<random>(ctx);
    }

    void register_random(script_context * state)
    {
      state->new_usertype<random>("random"
        , "integer", &random::integer
        , "real", &random::real
      );
      state->set_function("random_from_seed",[state](std::string const& seed)
      {
        return random_from_seed(state, seed);
      });

      state->set_function("random_from_time",[state]()
      {
        return random_from_time(state);
      });
    }
  } // namespace scripting
} // namespace noggit
