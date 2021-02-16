#pragma once

#include <vector>
#include <string>
#include <memory>
#include <FastNoise/FastNoise.h>

struct duk_hthread;

namespace noggit {
    namespace scripting {
        class script_noise_2d {
        public:
            script_noise_2d(unsigned start_x, unsigned start_y, unsigned width, unsigned height);
            float get(int x, int y);
            float get_f(float x, float y);
            void set(int x, int y, float value);
            unsigned get_start_x();
            unsigned get_start_y();
            unsigned get_width();
            unsigned get_height();
            std::vector<float> _noise;
        private:
            unsigned _width, _height, _start_x, _start_y;
        };

        class script_noise_generator {
        public:
            script_noise_generator(FastNoise::SmartNode<> generator);
            std::shared_ptr<script_noise_2d> uniform_2d(std::string seed, int xStart, int yStart, unsigned xSize, unsigned ySize, float frequency);
        private:
            FastNoise::SmartNode<> _generator;
        };

        std::shared_ptr<script_noise_generator> noise_simplex();
        std::shared_ptr<script_noise_generator> noise_perlin();
        std::shared_ptr<script_noise_generator> noise_value();
        std::shared_ptr<script_noise_generator> noise_fractal();
        std::shared_ptr<script_noise_generator> noise_cellular();
        std::shared_ptr<script_noise_generator> noise_white();
        std::shared_ptr<script_noise_generator> noise_custom(std::string encodedNodeTree);

        void register_noise_functions(duk_hthread* ctx);
    }
}