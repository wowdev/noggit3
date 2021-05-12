// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_profiles.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_settings.hpp>
#include <noggit/scripting/script_context.hpp>

// TODO: duplicate
#define CUR_PROFILE_PATH "__cur_profile"

namespace noggit
{
  namespace scripting
  {
    script_profiles::script_profiles(noggit::scripting::scripting_tool * tool)
        : QGroupBox("Script Profiles"), _tool(tool)
    {
      _select_column = new QGridLayout(this);

      _selection = new QComboBox(this);
      _selection->addItem("Default");
      _remove_button = new QPushButton("X", this);
      _name_entry = new QLineEdit(this);
      _create_button = new QPushButton("Add", this);

      _select_column->addWidget(_selection, 0, 0);
      _select_column->addWidget(_remove_button, 0, 1);

      _select_column->addWidget(_name_entry, 1, 0);
      _select_column->addWidget(_create_button, 1, 1);

      connect(_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        select_profile_gui(index);
      });

      connect(_remove_button, &QPushButton::released, this, [this]() {
        auto script_name = _tool->get_context()->get_selected_name();
        if (script_name.size() == 0)
        {
          // TODO: error?
          return;
        }

        auto index = _selection->currentIndex();

        // do not allow deleting default settings
        if (index == 0)
        {
          return;
        }

        auto text = _selection->itemText(index).toStdString();
        _selection->removeItem(index);
        
        auto json = _tool->get_settings()->get_raw_json();

        if (json->contains(script_name))
        {
          if ((*json)[script_name].contains(text))
          {
            (*json)[script_name].erase(text);
          }
        }

        select_profile_gui(0);
      });

      connect(_create_button, &QPushButton::released, this, [this]() {
        auto script_name = _tool->get_context()->get_selected_name();

        // do not allow invalid script
        if (script_name.size() == 0)
          return;

        auto newText = _name_entry->text();

        // do not allow empty profiles
        if (newText.isEmpty())
          return;

        auto count = _selection->count();
        for (int i = 0; i < count; ++i)
        {
          // do not allow duplicate profiles
          if (_selection->itemText(i) == newText)
          {
            return;
          }
        }

        _name_entry->clear();
        _selection->addItem(newText);

        auto json = _tool->get_settings()->get_raw_json();

        if (!(*json)[script_name].contains(newText.toStdString()))
        {
          if ((*json)[script_name].contains(_cur_profile))
          {
            (*json)[script_name][newText.toStdString()] = (*json)[script_name][_cur_profile];
          }
        }

        _selection->setCurrentIndex(count);

        select_profile_gui(count);
      });
    }

    void script_profiles::select_profile_gui(int profile)
    {
      select_profile(profile);
      _tool->clearDescription();
      _tool->get_context()->select_script(_tool->get_context()->get_selection());
      // string array / brush settings have changed
      _tool->get_settings()->initialize();
    }

    void script_profiles::select_profile(int profile)
    {
      _tool->get_settings()->clear();
      _cur_profile = _selection->itemText(profile).toStdString();
      auto n = _tool->get_context()->get_selected_name();
      _selection->setCurrentIndex(profile);
      (*_tool->get_settings()->get_raw_json())[n][CUR_PROFILE_PATH] = _cur_profile;
    }

    std::string script_profiles::get_cur_profile()
    {
      return _cur_profile;
    }

    void script_profiles::clear()
    {
      _selection->clear();
    }

    void script_profiles::add_profile(std::string const& profile)
    {
      _selection->addItem(QString::fromStdString (profile));
    }

    int script_profiles::profile_count()
    {
      return _selection->count();
    }

    std::string script_profiles::get_profile(int index)
    {
      return _selection->itemText(index).toStdString();
    }
  } // namespace scripting
} // namespace noggit