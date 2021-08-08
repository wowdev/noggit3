// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_brush.hpp>
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
#include <QSettings>

#include <map>
#include <mutex>

class World;
class MapView;

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
      scripting_tool(QWidget* parent
                    , MapView* view
                    , QSettings * noggit_settings
                    );
      ~scripting_tool();

      void addDescription(std::string const& text);
      void clearDescription();
      void addLog(std::string const& text);
      void resetLogScroll();
      void clearLog();
      void doReload();
      void sendBrushEvent(math::vector_3d const& pos,float dt);

      MapView* get_view();
      script_context* get_context();
      script_settings* get_settings();
      script_profiles* get_profiles();
      QSettings* get_noggit_settings();

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
      std::unique_ptr<script_context> _script_context = nullptr;
      MapView* _view;
      QSettings * _noggit_settings;
      void change_script(int script_index);
    };

    // TEMP: remove when exceptions are working
    void set_cur_exception(std::string const& exception);
  } // namespace scripting
} // namespace noggit
