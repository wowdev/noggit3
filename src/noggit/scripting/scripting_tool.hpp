#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>

#include <math/trig.hpp>
#include <math/vector_3d.hpp>
#include <noggit/tool_enums.hpp>

#include <noggit/scripting/script_setup.hpp>
#include <noggit/camera.hpp>

class World;

// The visible GUI, managed and invoked by the rest of noggit.

namespace noggit
{
    namespace scripting
    {
        class scripting_tool : public QWidget
        {
        public:
            scripting_tool(QWidget *parent = nullptr);
            float brushRadius() const { return _radius; }
            float innerRadius() const { return _inner_radius; }

            void addDescription(std::string text);
            void clearDescription();
            void addLog(std::string text);
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

            double_holder *addDouble(std::string name, double min, double max, double def = 0);
            int_holder *addInt(std::string name, int min, int max, int def = 0);
            string_holder *addString(std::string name, std::string def = "");
            bool_holder *addBool(std::string name, bool def = false);

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

            std::vector<QWidget *> _script_widgets;
            std::vector<void *> _holders;
        };

        scripting_tool* get_cur_tool();
    } // namespace ui
} // namespace noggit