#pragma once

#include <boost/filesystem.hpp>
#include <string>

struct duk_hthread;

namespace noggit {
    namespace scripting {
        namespace fs = boost::filesystem;

        class script_file_iterator {
        public:
            script_file_iterator(fs::recursive_directory_iterator dir, fs::recursive_directory_iterator end);
            bool is_on_file();
            std::string get_file();
            void next_file();
        private:
            fs::recursive_directory_iterator _dir;
            fs::recursive_directory_iterator _end;
            void skip_dirs();
        };

        void write_file(std::string path, std::string content);
        void append_file(std::string path, std::string content);
        std::shared_ptr<script_file_iterator> read_directory(std::string path);
        std::string read_file(std::string path);
        bool path_exists(std::string path);

        void register_filesystem_functions(duk_hthread *ctx);

    }
}