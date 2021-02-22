// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <FastNoise/FastNoise.h>
#include <noggit/scripting/script_selection.hpp>
#include <math/vector_3d.hpp>

namespace noggit
{
    namespace scripting
    {
        struct script_noise_map
        {
            script_noise_map() {}
            std::vector<float> _noise;
            unsigned _width = 0;
            unsigned _height = 0;
            unsigned _start_x = 0;
            unsigned _start_y = 0;
            void resize(unsigned width, unsigned height, unsigned start_x, unsigned start_y);
        };

        float noise_get_index(script_noise_map &noise, int x, int y);
        float noise_get_global(script_noise_map &noise, math::vector_3d &pos);
        bool noise_is_highest_global(script_noise_map &noise, math::vector_3d &pos, int check_radius);
        void noise_set(script_noise_map &noise, int x, int y, float value);
        unsigned noise_start_x(script_noise_map &noise);
        unsigned noise_start_y(script_noise_map &noise);
        unsigned noise_width(script_noise_map &noise);
        unsigned noise_height(script_noise_map &noise);

        struct script_noise_generator
        {
            script_noise_generator(FastNoise::SmartNode<> generator);
            script_noise_generator() {}
            std::shared_ptr<script_noise_map> uniform_2d(std::string seed, int xStart, int yStart, unsigned xSize, unsigned ySize, float frequency);
            FastNoise::SmartNode<> _generator;
        };

        script_noise_map make_noisemap();

        void noise_fill(script_noise_generator &thiz, script_noise_map &noisemap, const char *seed, int xStart, int yStart, unsigned xSize, unsigned ySize, float frequency);
        void noise_fill_selection(script_noise_generator &thiz, script_noise_map &noisemap, script_selection &selection, const char *seed, float frequency, int padding);

        script_noise_generator make_noisegen_simplex();
        script_noise_generator make_noisegen_perlin();
        script_noise_generator make_noisegen_value();
        script_noise_generator make_noisegen_fractal();
        script_noise_generator make_noisegen_cellular();
        script_noise_generator make_noisegen_white();
        script_noise_generator make_noisegen_custom(const char *encodedNodeTree);
    } // namespace scripting
} // namespace noggit