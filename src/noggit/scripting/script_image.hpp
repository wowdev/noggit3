#pragma once

#include <vector>
#include <string>

#include <memory>

struct duk_hthread;

namespace noggit {
    namespace scripting {
        class script_image
        {
        public:
            script_image(std::string path);
            script_image(unsigned width, unsigned height);
            unsigned get_index(unsigned x, unsigned y);
            unsigned get_pixel(unsigned x, unsigned y);
            void set_pixel(unsigned x, unsigned y, unsigned value);
            void save(std::string filename);

            unsigned get_width();
            unsigned get_height();
        private:
            std::vector<unsigned char> image;
            unsigned width;
            unsigned height;
        };

        std::shared_ptr<script_image> load_image(std::string path);
        std::shared_ptr<script_image> create_image(unsigned width, unsigned height);

        void register_image_functions(duk_hthread* ctx);
    }
}
