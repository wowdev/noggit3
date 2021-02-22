// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_random.hpp>

using namespace noggit::scripting;

noggit::scripting::script_random::script_random(unsigned seed)
    : _state(seed)
{
}

noggit::scripting::script_random::script_random(std::string seed)
    : _state(std::hash<std::string>()(seed))
{
}

noggit::scripting::script_random::script_random()
    : _state(std::chrono::high_resolution_clock::now().time_since_epoch().count())
{
}

int noggit::scripting::rand_int32(script_random &rand, int low, int high)
{
    return low + int(rand_uint64(rand, 0, std::abs(high - low)));
}

unsigned noggit::scripting::rand_uint32(script_random &rand, unsigned low, unsigned high)
{
    return rand_uint64(rand, low, high);
}

unsigned long noggit::scripting::rand_uint64(script_random &rand, unsigned long low, unsigned long high)
{
    auto x = rand._state;
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    rand._state = x;
    // does the modulo bias cancel out the use of 64 bits here?
    return low + ((x * 0x2545F4914F6CDD1DULL) % (high - low));
}

long noggit::scripting::rand_int64(script_random &rand, long low, long high)
{
    return low + rand_uint64(rand, 0, std::abs(high - low));
}

double noggit::scripting::rand_double(script_random &rand, double low, double high)
{
#define RAND_MAX_DOUBLE 18446744073709551615ull
    return low + rand_uint64(rand, 0, RAND_MAX_DOUBLE) / (RAND_MAX_DOUBLE / (high - low));
}

float noggit::scripting::rand_float(script_random &rand, float low, float high)
{
// TODO: prolly not a good idea
#define RAND_MAX_FLOAT 4294967295
    return low + rand_uint32(rand, 0, RAND_MAX_FLOAT) / (RAND_MAX_FLOAT / (high - low));
}

script_random noggit::scripting::random_from_seed(const char *seed)
{
    return script_random(std::string(seed));
}

script_random noggit::scripting::random_from_time()
{
    return script_random();
}