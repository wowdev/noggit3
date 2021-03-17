// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_context.hpp>
#include <noggit/tool_enums.hpp>

#include <math/trig.hpp>
#include <math/vector_3d.hpp>

#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>

#include <map>
#include <mutex>

class World;

namespace noggit
{
  class camera;
  namespace scripting
  {
    class script_context;
    class script_settings;
    class script_profiles;
    class scripting_tool : public QWidget
    {
    public:
      scripting_tool(QWidget* parent, World* world, noggit::camera* camera);
      ~scripting_tool();

      void addDescription(char const* text);
      void clearDescription();
      void addLog(std::string const& text);
      void resetLogScroll();
      void clearLog();
      void doReload();
      void sendUpdate(
        World* world,
        math::vector_3d pos,
        noggit::camera* cam,
        float dt,
        bool leftButton,
        bool rightButton,
        bool holding_shift,
        bool holding_ctrl,
        bool holding_alt,
        bool holding_space);

      World* get_world();
      noggit::camera* get_camera();
      script_context* get_context();
      script_settings* get_settings();
      script_profiles* get_profiles();

    private:
      std::mutex _script_change_mutex;
      std::string _cur_profile;

      bool _last_left = false;
      bool _last_right = false;

    private:
      QComboBox* _selection;
      QPushButton* _reload_button;

      QLabel* _description;
      QPlainTextEdit* _log;

      script_settings *_settings;
      script_profiles *_profiles;
    private:
      script_context _script_context;
      World *_world;
      noggit::camera *_camera;
      void change_script(int script_index);
    };
  } // namespace scripting
} // namespace noggit
