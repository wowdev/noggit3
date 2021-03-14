// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <QtCore/QString>

#include <string>

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;

    struct Loader
    {
      int load_scripts(scripting_tool*);

      void send_left_click (scripting_tool* tool);
      void send_left_hold (scripting_tool* tool);
      void send_left_release (scripting_tool* tool);

      void send_right_click (scripting_tool* tool);
      void send_right_hold (scripting_tool* tool);
      void send_right_release (scripting_tool* tool);

      int script_count();
      std::string selected_script_name();
      std::string const& get_script_name(int id);
      QString get_script_display_name(int id);
      void select_script(int index, scripting_tool*);
      int get_selected_script();

    private:
      struct script_container
      {
        script_container(
            std::string name,
            QString display_name,
            std::unique_ptr<das::Context> ctx,
            bool select,
            bool left_click,
            bool left_hold,
            bool left_release,
            bool right_click,
            bool right_hold,
            bool right_release);
        script_container() = default;

        std::unique_ptr<das::Context> _ctx;

        std::string _name;
        QString _display_name;

        bool _on_select = false;

        bool _on_left_click = false;
        bool _on_left_hold = false;
        bool _on_left_release = false;

        bool _on_right_click = false;
        bool _on_right_hold = false;
        bool _on_right_release = false;
      };

      std::vector<script_container> containers;
      int cur_script = -1;
    };
  } // namespace scripting
} // namespace noggit
