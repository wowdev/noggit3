// This file is part of the Script Brushes extension of Noggit3 by TSWoW (https://github.com/tswow/)
// licensed under GNU General Public License (version 3).
#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton.h>
#include <QtWidgets/QGridLayout.h>

#include <math/trig.hpp>
#include <math/vector_3d.hpp>
#include <noggit/tool_enums.hpp>

#include <map>

class World;

// The visible GUI, managed and invoked by the rest of noggit.

namespace noggit
{
    class camera;
    namespace scripting
    {
        class scripting_tool : public QWidget
        {
        public:
            scripting_tool(QWidget *parent = nullptr);
            float brushRadius() const { return _radius; }
            float innerRadius() const { return _inner_radius; }

            void addDescription(const char *text);
            void clearDescription();
            void addLog(const std::string &text);
            void clearLog();
            void doReload();
            void sendUpdate(
                World *world,
                math::vector_3d pos,
                noggit::camera *cam,
                float dt,
                bool leftButton,
                bool rightButton,
                bool holding_shift,
                bool holding_ctrl,
                bool holding_alt,
                bool holding_space);

            void addDouble(const char *name, double min, double max, double def = 0, int zeros = 2);
            void addInt(const char *name, int min, int max, int def = 0);
            void addBool(const char *name, bool def = false);
            void addString(const char *name, const char *def = "");

            void addStringList(const char *name, const char *value);

            void removeScriptWidgets();

        private:
            bool _last_left = false;
            bool _last_right = false;

            float _radius = 0;
            float _speed = 0;
            float _inner_radius = 0;

            QComboBox *script_selection;

            QDoubleSpinBox *_radius_spin;
            QSlider *_radius_slider;

            QDoubleSpinBox *_inner_radius_spin;
            QSlider *_inner_radius_slider;

            QLabel *_description;
            QPlainTextEdit *_log;

            QGroupBox *_script_settings_group;
            QFormLayout *_script_settings_layout;

            QGroupBox *_profile_group;
            QFormLayout *_profile_layout;
            QComboBox *_profile_selection;
            QLineEdit *_profile_name_entry;
            QLabel *_profile_remove_label;
            QPushButton *_profile_remove_button;
            QPushButton *_profile_create_button;

            QGridLayout *_profile_select_column;

            std::vector<QWidget *> _script_widgets;
            std::vector<void *> _holders;

            std::map<std::string, QComboBox *> _string_arrays;

            void select_profile(int profile);
            void on_change_script(int script_index);
            void initialize_radius();
        };

        const char *get_string_param(const char *path);
        int get_int_param(const char *path);
        double get_double_param(const char *path);
        float get_float_param(const char *path);
        bool get_bool_param(const char *path);
        const char *get_string_list_param(const char *path);

        void add_string_list_param(const char *path, const char *value);
        void add_string_param(const char *path, const char *def);
        void add_int_param(const char *path, int min, int max, int def);
        void add_double_param(const char *path, double min, double max, double def, int zeros);
        void add_float_param(const char *path, float min, float max, float def, int zeros);
        void add_bool_param(const char *path, bool def);
        void add_description(const char *desc);

        void save_json();
        scripting_tool *get_cur_tool();
    } // namespace scripting
} // namespace noggit