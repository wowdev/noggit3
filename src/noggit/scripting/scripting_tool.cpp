// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <cmath>

#include <noggit/camera.hpp>
#include <noggit/Log.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/scripting/script_profiles.hpp>
#include <noggit/scripting/script_settings.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QCheckBox>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#define CUR_PROFILE_PATH "__cur_profile"

namespace noggit
{
  namespace scripting
  {
    void scripting_tool::doReload()
    {
      get_settings()->clear();
      clearLog();
      try
      {
        get_context()->reset(this);
      }
      catch (std::exception const& e)
      {
        addLog("[error]: " + std::string(e.what()));
        resetLogScroll();
        return;
      }
      int selection = get_context()->get_selection();

      _selection->clear();

      for(auto& script : get_context()->get_scripts())
      {
        _selection->addItem(script.get_name().c_str());
      }

      if (selection != -1)
      {
        _selection->setCurrentIndex(selection);
        change_script(selection);
      }
    }

    void scripting_tool::change_script(int selection)
    {
      std::lock_guard<std::mutex> const lock (_script_change_mutex);

      clearDescription();
      get_settings()->clear();

      auto sn = _script_context.get_scripts()[selection].get_name();

      get_profiles()->clear();

      auto json = get_settings()->get_raw_json();

      if (json->contains(sn))
      {
        std::vector<std::string> items;
        for (auto& v : (*json)[sn].items())
        {
          if (v.key() != CUR_PROFILE_PATH)
          {
            items.push_back(v.key());
          }
        }

        std::sort(items.begin(), items.end(), [](auto a, auto b) {
          if (a == "Default")
            return true;
          if (b == "Default")
            return false;
          return a < b;
        });

        for (auto& item : items)
        {
          get_profiles()->add_profile(item);
        }
      }

      if (get_profiles()->profile_count() == 0)
      {
        get_profiles()->add_profile("Default");
      }

      int next_profile = 0;
      auto cur_script = get_context()->get_scripts()[selection].get_name();
      if (json->contains(cur_script))
      {
        if ((*json)[cur_script].contains(CUR_PROFILE_PATH))
        {
          auto str = (*json)[cur_script][CUR_PROFILE_PATH];
          for (int i = 0; i < get_profiles()->profile_count(); ++i)
          {
            if (get_profiles()->get_profile(i) == str)
            {
              next_profile = i;
              break;
            }
          }
        }
      }

      get_profiles()->set_profile(next_profile);
      get_context()->select_script(selection);
      get_settings()->initialize();
    }

    scripting_tool::scripting_tool(QWidget* parent, World * world, noggit::camera * camera)
      : QWidget(parent)
      , _cur_profile ("Default")
      , _world(world)
      , _camera(camera)
    {
      auto layout(new QVBoxLayout(this));
      _selection = new QComboBox();
      layout->addWidget(_selection);

      _reload_button = new QPushButton("Reload Scripts", this);
      layout->addWidget(_reload_button);
      connect(_reload_button, &QPushButton::released, this, [this]() {
        doReload();
      });

      _profiles = new script_profiles(this);
      layout->addWidget(_profiles);

      _settings = new script_settings(this);
      _settings->load_json();
      layout->addWidget(_settings);

      _description = new QLabel(this);
      layout->addWidget(_description);

      _log = new QPlainTextEdit(this);
      _log->setFont (QFontDatabase::systemFont (QFontDatabase::FixedFont));
      _log->setReadOnly(true);
      layout->addWidget(_log);

      connect(_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        clearLog();
        change_script(index);
      });

      doReload();
    }

    scripting_tool::~scripting_tool()
    {
      get_settings()->save_json();
     }

    void scripting_tool::sendUpdate(
        World* world,
        math::vector_3d pos_in,
        noggit::camera* cam,
        float dt,
        bool left_mouse,
        bool right_mouse,
        bool holding_shift,
        bool holding_ctrl,
        bool holding_alt,
        bool holding_space)
    {
      //script_context ctx(pos_in, brushRadius(), innerRadius(), cam, holding_alt, holding_shift, holding_ctrl, holding_space, dt);
      try
      {
        // TODO: fix
      }
      catch (std::exception const& e)
      {
        doReload();
        addLog(("[error]: " + std::string(e.what())));
        resetLogScroll();
      }
      _last_left = left_mouse;
      _last_right = right_mouse;
    }

    void scripting_tool::addDescription(char const* text)
    {
      std::string stext = text == nullptr ? "" : text;
      _description->setText(_description->text() + "\n" + QString::fromStdString (stext));
    }

    void scripting_tool::addLog(std::string const& text)
    {

      LogDebug << "[script window]: " << text << "\n";
      _log->appendPlainText (QString::fromStdString (text));
      _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
    }

    script_context* scripting_tool::get_context()
    {
      return &_script_context;
    }

    World* scripting_tool::get_world()
    {
      return _world;
    }

    noggit::camera* scripting_tool::get_camera()
    {
      return _camera;
    }

    void scripting_tool::resetLogScroll()
    {
      _log->verticalScrollBar()->setValue(0);
    }

    void scripting_tool::clearLog()
    {
      _log->clear();
    }

    void scripting_tool::clearDescription()
    {
      _description->clear();
    }

    script_settings* scripting_tool::get_settings()
    {
      return _settings;
    }

    script_profiles* scripting_tool::get_profiles()
    {
      return _profiles;
    }

  } // namespace scripting
} // namespace noggit
