#pragma once

#include <random>
#include <chrono>
#include <memory>

struct duk_hthread;

namespace noggit {
    namespace scripting {
        // TODO: are you supposed to re-create the distributions each time you need them?
        class script_random {
            public:
            script_random(unsigned seed);
            script_random(std::string seed);
            script_random();

            int get_int32(int low, int high);
            unsigned get_uint32(unsigned low, unsigned high);
            unsigned long get_uint64(unsigned long low, unsigned long high);
            long get_int64(long low, long high);
            double get_double(double low, double high);
            double get_float(float low, float high);

            private:
            std::mt19937 _engine;
        };

        std::shared_ptr<script_random> random_from_seed(std::string seed);
        std::shared_ptr<script_random> random_from_time();

        void register_random_functions(duk_hthread* ctx);
    }
}