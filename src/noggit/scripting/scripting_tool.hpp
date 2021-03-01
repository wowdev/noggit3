// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

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
#include <QtWidgets/QPushButton.h>
#include <QtWidgets/QGridLayout.h>

#include <map>

class World;

namespace noggit
{
  class camera;
  namespace scripting
  {
    class scripting_tool : public QWidget
    {
    public:
      scripting_tool(QWidget* parent = nullptr);
      float brushRadius() const { return _radius; }
      float innerRadius() const { return _inner_radius; }

      void addDescription(char const* text);
      void clearDescription();
      void addLog(std::string const& text);
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

      void addDouble(char const* name, double min, double max, double def = 0, int zeros = 2);
      void addInt(char const* name, int min, int max, int def = 0);
      void addBool(char const* name, bool def = false);
      void addString(char const* name, char const* def = "");

      void addStringList(char const* name, char const* value);

      void removeScriptWidgets();

    private:
      bool _last_left = false;
      bool _last_right = false;

      float _radius = 0;
      float _speed = 0;
      float _inner_radius = 0;

    private:
      QComboBox* _script_selection;
      QPushButton* _reload_button;

      QGroupBox* _radius_group;
      QFormLayout* _radius_layout;
      QDoubleSpinBox* _radius_spin;
      QSlider* _radius_slider;

      QDoubleSpinBox* _inner_radius_spin;
      QSlider* _inner_radius_slider;

      QLabel* _description;
      QPlainTextEdit* _log;

      QGroupBox* _script_settings_group;
      QFormLayout* _script_settings_layout;

      QGroupBox* _profile_group;
      QFormLayout* _profile_layout;
      QComboBox* _profile_selection;
      QLineEdit* _profile_name_entry;
      QLabel* _profile_remove_label;
      QPushButton* _profile_remove_button;
      QPushButton* _profile_create_button;

      QGridLayout* _profile_select_column;

      std::vector<QWidget*> _script_widgets;
      std::vector<void*> _holders;

      std::map<std::string, QComboBox*> _string_arrays;

      void select_profile(int profile);
      void on_change_script(int script_index);
      void initialize_radius();
    };

    char const* get_string_param(char const* path);
    int get_int_param(char const* path);
    double get_double_param(char const* path);
    float get_float_param(char const* path);
    bool get_bool_param(char const* path);
    char const* get_string_list_param(char const* path);

    void add_string_list_param(char const* path, char const* value);
    void add_string_param(char const* path, char const* def);
    void add_int_param(char const* path, int min, int max, int def);
    void add_double_param(char const* path, double min, double max, double def, int zeros);
    void add_float_param(char const* path, float min, float max, float def, int zeros);
    void add_bool_param(char const* path, bool def);
    void add_description(char const* desc);

    void save_json();

    // same as script_context, but must also be accessable during "select"
    scripting_tool* get_cur_tool();
  } // namespace scripting
} // namespace noggit
