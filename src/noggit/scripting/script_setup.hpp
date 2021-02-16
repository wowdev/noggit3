#pragma once

#include <string>

struct duk_hthread;

namespace noggit
{
    namespace scripting
    {
        // Do not template these, we must expose their functions to dukglue
        class bool_holder
        {
            bool value;

        public:
            void set(bool value) { this->value = value; }
            bool get() { return value; }
        };

        class string_holder
        {
            std::string value;

        public:
            void set(std::string value) { this->value = value; }
            std::string get() { return value; }
        };

        class int_holder
        {
            int value;

        public:
            void set(int value) { this->value = value; }
            int get() { return value; }
        };

        class double_holder
        {
            double value;

        public:
            void set(double value) { this->value = value; }
            double get() { return value; }
        };

        // Startup functions
        int add_script(duk_hthread *ctx);
        void add_desc(std::string description);
        double_holder* param_double(std::string name, double min, double max, double def);
        int_holder* param_int(std::string name, int min, int max, int def);
        bool_holder* param_bool(std::string name, bool def = false);
        string_holder* param_string(std::string name, std::string def);

        void register_setup_functions(duk_hthread* ctx);
    } // namespace scripting
} // namespace noggit