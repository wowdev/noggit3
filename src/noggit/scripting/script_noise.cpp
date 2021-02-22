// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_noise.hpp>
#include <noggit/scripting/scripting_tool.hpp>

using namespace noggit::scripting;

script_noise_map noggit::scripting::make_noisemap()
{
    return script_noise_map();
}

float noggit::scripting::noise_get_index(script_noise_map &noise, int x, int y)
{
    unsigned index = x + y * noise._width;
    return noise._noise[index];
}

float noggit::scripting::noise_get_global(script_noise_map &noise, math::vector_3d &pos)
{
    return noise_get_index(noise, std::round(pos.x) - noise._start_x, std::round(pos.z) - noise._start_y);
}

bool noggit::scripting::noise_is_highest_global(script_noise_map &noise, math::vector_3d &pos, int check_radius)
{
    int x = std::round(pos.x) - noise._start_x;
    int z = std::round(pos.z) - noise._start_y;

    float own = noise_get_index(noise, x, z);

    for (int xc = x - check_radius; xc < x + check_radius; ++xc)
    {
        for (int zc = z - check_radius; zc < z + check_radius; ++zc)
        {
            if (xc == x && zc == z)
            {
                continue;
            }

            if (noise_get_index(noise, xc, zc) > own)
            {
                return false;
            }
        }
    }
    return true;
}

void noggit::scripting::noise_set(script_noise_map &noise, int x, int y, float value)
{
    unsigned index = x + y * noise._width;
    noise._noise[index] = value;
}

void noggit::scripting::script_noise_map::resize(unsigned width, unsigned height, unsigned start_x, unsigned start_y)
{
    _width = width;
    _height = height;
    _start_x = _start_x;
    _start_y = _start_y;
    _noise.resize(width * height);
}

unsigned noggit::scripting::noise_start_x(script_noise_map &noise) { return noise._start_x; }
unsigned noggit::scripting::noise_start_y(script_noise_map &noise) { return noise._start_y; }
unsigned noggit::scripting::noise_width(script_noise_map &noise) { return noise._width; }
unsigned noggit::scripting::noise_height(script_noise_map &noise) { return noise._height; }

noggit::scripting::script_noise_generator::script_noise_generator(FastNoise::SmartNode<> generator)
    : _generator(generator) {}

void noggit::scripting::noise_fill(script_noise_generator &thiz, script_noise_map &map, const char *seed, int x_start, int y_start, unsigned x_size, unsigned y_size, float frequency)
{
    map._noise.resize(x_size * y_size);
    map._start_x = x_start;
    map._start_y = y_start;
    map._width = x_size;
    map._height = y_size;
    thiz._generator->GenUniformGrid2D(map._noise.data(), x_start, y_start, x_size, y_size, frequency, std::hash<std::string>()(std::string(seed)));
}

void noggit::scripting::noise_fill_selection(script_noise_generator &thiz, script_noise_map &map, script_selection &selection, const char *seed, float frequency, int padding)
{
    auto x_start = std::floor(selection._min.x) - (padding + 1);
    auto z_start = std::floor(selection._min.z) - (padding + 1);

    auto x_size = std::ceil(selection._max.x - selection._min.x) + (padding + 1) * 2;
    auto z_size = std::ceil(selection._max.z - selection._min.z) + (padding + 1) * 2;

    noise_fill(thiz, map, seed, x_start, z_start, x_size, z_size, frequency);
}

script_noise_generator noggit::scripting::make_noisegen_simplex()
{
    return script_noise_generator(FastNoise::New<FastNoise::Simplex>());
}
script_noise_generator noggit::scripting::make_noisegen_perlin()
{
    return script_noise_generator(FastNoise::New<FastNoise::Perlin>());
}
script_noise_generator noggit::scripting::make_noisegen_value()
{
    return script_noise_generator(FastNoise::New<FastNoise::Value>());
}
script_noise_generator noggit::scripting::make_noisegen_fractal()
{
    return script_noise_generator(FastNoise::New<FastNoise::FractalFBm>());
}
script_noise_generator noggit::scripting::make_noisegen_cellular()
{
    return script_noise_generator(FastNoise::New<FastNoise::CellularValue>());
}
script_noise_generator noggit::scripting::make_noisegen_white()
{
    return script_noise_generator(FastNoise::New<FastNoise::White>());
}
script_noise_generator noggit::scripting::make_noisegen_custom(const char *encodedNodeTree)
{
    return script_noise_generator(FastNoise::NewFromEncodedNodeTree(encodedNodeTree));
}