// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/SettingsPanel.h>

#include <noggit/TextureManager.h>
#include <util/qt/overload.hpp>


#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>


#include <algorithm>

namespace util
{
  file_line_edit::file_line_edit (mode m, QString browse_title, QWidget* parent)
    : QWidget (parent)
  {
    new QHBoxLayout (this);
    layout()->setContentsMargins (0, 0, 0, 0);

    layout()->addWidget (actual = new QLineEdit);
    auto button (new QPushButton ("Browse…", this));
    layout()->addWidget (button);

    connect ( button, &QPushButton::clicked
            , [=]
              {
                auto result
                  ( m == files
                  ? QFileDialog::getOpenFileName
                      (nullptr, browse_title, actual->text())
                  : QFileDialog::getExistingDirectory
                      (nullptr, browse_title, actual->text())
                  );
                if (!result.isNull())
                {
                  if (m == directories && !(result.endsWith ("/") || result.endsWith ("\\")))
                  {
                    result += "/";
                  }
                  actual->setText (result);
                }
              }
            );
  }
}

namespace noggit
{
  namespace ui
  {
    settings::settings(QWidget* parent)
      : QDialog (parent)
      , _settings (new QSettings (this))
    {
      setWindowIcon (QIcon (":/icon"));
      setWindowTitle ("Settings");

      auto layout (new QFormLayout (this));

      auto browse_row
        ( [&] (util::file_line_edit** line, char const* title, QString const& key, util::file_line_edit::mode mode)
          {
            layout->addRow
              ( title
              , *line = new util::file_line_edit (mode, title, this)
              );
            connect ( (*line)->actual, &QLineEdit::textChanged
                    , [&, key] (QString value)
                      {
                        _settings->setValue (key, value);
                      }
                    );
          }
        );


      browse_row (&gamePathField, "Game Path", "project/game_path", util::file_line_edit::directories);
      browse_row (&projectPathField, "Project Path", "project/path", util::file_line_edit::directories);
      browse_row (&importPathField, "Import Path", "project/import_file", util::file_line_edit::files);
      browse_row (&wmvLogPathField, "WMV Log Path", "project/wmv_log_file", util::file_line_edit::files);      

      _mysql_box = new QGroupBox ("MySQL (uid storage)", this);
      _mysql_box->setToolTip ("Store the maps' max model unique id (uid) in a mysql database to sync your uids with different computers/users to avoid duplications");
      auto mysql_layout (new QFormLayout (_mysql_box));

#ifdef USE_MYSQL_UID_STORAGE
      mysql_box->setCheckable (true);

      _mysql_server_field = new QLineEdit(_settings->value("project/mysql/server").toString(), this);
      _mysql_user_field = new QLineEdit(_settings->value("project/mysql/user").toString(), this);
      _mysql_pwd_field = new QLineEdit(_settings->value("project/mysql/pwd").toString(), this);
      _mysql_db_field = new QLineEdit(_settings->value("project/mysql/db").toString(), this);

      mysql_layout->addRow("Server", _mysql_server_field);
      mysql_layout->addRow("User", _mysql_user_field);
      mysql_layout->addRow("Password", _mysql_pwd_field);
      mysql_layout->addRow("Database", _mysql_db_field);
#else
      mysql_layout->addRow (new QLabel ("Your noggit wasn't build with mysql, you can't use this feature"));
#endif

      layout->addRow (_mysql_box);

      auto wireframe_box (new QGroupBox ("Wireframe", this));
      auto wireframe_layout (new QFormLayout (wireframe_box));

      _wireframe_type_group = new QButtonGroup (wireframe_box);

      auto radio_wire_full (new QRadioButton ("Full wireframe"));
      auto radio_wire_cursor (new QRadioButton ("Around cursor"));

      _wireframe_type_group->addButton (radio_wire_full, 0);
      _wireframe_type_group->addButton (radio_wire_cursor, 1);     

      wireframe_layout->addRow (new QLabel ("Type:"));
      wireframe_layout->addRow (radio_wire_full);
      wireframe_layout->addRow (radio_wire_cursor);

      _wireframe_radius = new QDoubleSpinBox (wireframe_box);
      _wireframe_radius->setRange (1.0, 100.0);

      wireframe_layout->addRow ("Radius", _wireframe_radius);
      wireframe_layout->addRow (new QLabel ("(real radius = cursor radius * wireframe radius)"));

      _wireframe_width = new QDoubleSpinBox (wireframe_box);
      _wireframe_width->setRange (0.0, 10.0);
      _wireframe_width->setSingleStep(0.1);
      wireframe_layout->addRow ("Width", _wireframe_width);

      wireframe_layout->addRow ("Color", _wireframe_color = new color_widgets::ColorSelector (wireframe_box));
      layout->addRow (wireframe_box);

      
      layout->addRow ("VSync", _vsync_cb = new QCheckBox (this));
      layout->addRow ("Anti Aliasing", _anti_aliasing_cb = new QCheckBox(this));
      layout->addRow ("Fullscreen", _fullscreen_cb = new QCheckBox(this));
      _vsync_cb->setToolTip("Require restart");
      _anti_aliasing_cb->setToolTip("Require restart");
      _fullscreen_cb->setToolTip("Require restart");

      layout->addRow ( "View Distance"
                     , viewDistanceField = new QDoubleSpinBox
                     );
      viewDistanceField->setRange (0.f, 1048576.f);      

      layout->addRow ( "FarZ"
                     , farZField = new QDoubleSpinBox
                     );
      farZField->setRange (0.f, 1048576.f);

      layout->addRow ( "Adt unloading distance (in adt)", _adt_unload_dist = new QSpinBox(this));
      _adt_unload_dist->setRange(1, 64);

      layout->addRow ("Adt unloading check interval (sec)", _adt_unload_check_interval = new QSpinBox(this));
      _adt_unload_check_interval->setMinimum(1);

      layout->addRow ("Always check for max UID", _uid_cb = new QCheckBox(this));

      layout->addRow ("Tablet support", tabletModeCheck = new QCheckBox(this));

      layout->addRow("Undock tool properties", _undock_tool_properties = new QCheckBox(this));
      layout->addRow("Undock quick access texture palette", _undock_small_texture_palette = new QCheckBox(this));

      layout->addRow("Additional file loading log", _additional_file_loading_log = new QCheckBox(this));

#ifdef NOGGIT_HAS_SCRIPTING
      layout->addRow("Allow scripts to write to any file",_allow_scripts_write_any_file = new QCheckBox(this));
#endif

      auto warning (new QWidget (this));
      new QHBoxLayout (warning);
      auto icon (new QLabel (warning));
      icon->setPixmap
        (render_blp_to_pixmap ("interface/gossipframe/availablequesticon.blp"));
      warning->layout()->addWidget (icon);
      warning->layout()->addWidget
        (new QLabel ("Changes may not take effect until next launch.", warning));
      layout->addRow (warning);

      auto buttonBox ( new QDialogButtonBox ( QDialogButtonBox::Save
                                            | QDialogButtonBox::Cancel
                                            )
                     );

      layout->addRow (buttonBox);

      connect ( buttonBox, &QDialogButtonBox::accepted
              , [this]
                {
                  hide();
                  save_changes();
                }
              );

      connect ( buttonBox, &QDialogButtonBox::rejected
              , [this]
                {
                  hide();
                  discard_changes();
                }
              );

      // load the values in the fields
      discard_changes();
    }

    void settings::discard_changes()
    {
      gamePathField->actual->setText (_settings->value ("project/game_path").toString());
      projectPathField->actual->setText (_settings->value ("project/path").toString());
      importPathField->actual->setText (_settings->value ("project/import_file").toString());
      wmvLogPathField->actual->setText (_settings->value ("project/wmv_log_file").toString());
      viewDistanceField->setValue (_settings->value ("view_distance", 1000.f).toFloat());
      farZField->setValue (_settings->value ("farZ", 2048.f).toFloat());
      tabletModeCheck->setChecked (_settings->value ("tablet/enabled", false).toBool());
      _undock_tool_properties->setChecked (_settings->value ("undock_tool_properties/enabled", true).toBool());
      _undock_small_texture_palette->setChecked (_settings->value ("undock_small_texture_palette/enabled", true).toBool());
      _vsync_cb->setChecked (_settings->value ("vsync", false).toBool());
      _anti_aliasing_cb->setChecked (_settings->value ("anti_aliasing", false).toBool());
      _fullscreen_cb->setChecked (_settings->value ("fullscreen", false).toBool());
      _adt_unload_dist->setValue(_settings->value("unload_dist", 5).toInt());
      _adt_unload_check_interval->setValue(_settings->value("unload_interval", 5).toInt());
      _uid_cb->setChecked(_settings->value("uid_startup_check", true).toBool());
      _additional_file_loading_log->setChecked(_settings->value("additional_file_loading_log", false).toBool());
#ifdef NOGGIT_HAS_SCRIPTING
      _allow_scripts_write_any_file->setChecked(_settings->value("allow_scripts_write_any_file",false).toBool());
#endif
#ifdef USE_MYSQL_UID_STORAGE
      _mysql_box->setChecked (_settings->value ("project/mysql/enabled").toBool());
      _mysql_server_field->setText (_settings->value ("project/mysql/server").toString());
      _mysql_user_field->setText(_settings->value ("project/mysql/user").toString());
      _mysql_pwd_field->setText (_settings->value ("project/mysql/pwd").toString());
      _mysql_db_field->setText (_settings->value ("project/mysql/db").toString());
#endif

      _wireframe_type_group->button (_settings->value ("wireframe/type", 0).toInt())->toggle();
      _wireframe_radius->setValue (_settings->value ("wireframe/radius", 1.5f).toFloat());
      _wireframe_width->setValue (_settings->value ("wireframe/width", 1.f).toFloat());
      _wireframe_color->setColor(_settings->value("wireframe/color").value<QColor>());      
    }

    void settings::save_changes()
    {
      _settings->setValue ("project/game_path", gamePathField->actual->text());
      _settings->setValue ("project/path", projectPathField->actual->text());
      _settings->setValue ("project/import_file", importPathField->actual->text());
      _settings->setValue ("project/wmv_log_file", wmvLogPathField->actual->text());
      _settings->setValue ("farZ", farZField->value());
      _settings->setValue ("view_distance", viewDistanceField->value());
      _settings->setValue ("tablet/enabled", tabletModeCheck->isChecked());
      _settings->setValue ("undock_tool_properties/enabled", _undock_tool_properties->isChecked());
      _settings->setValue ("undock_small_texture_palette/enabled", _undock_small_texture_palette->isChecked());
      _settings->setValue ("vsync", _vsync_cb->isChecked());
      _settings->setValue ("anti_aliasing", _anti_aliasing_cb->isChecked());
      _settings->setValue ("fullscreen", _fullscreen_cb->isChecked());
      _settings->setValue ("unload_dist", _adt_unload_dist->value());
      _settings->setValue ("unload_interval", _adt_unload_check_interval->value());
      _settings->setValue ("uid_startup_check", _uid_cb->isChecked());
      _settings->setValue ("additional_file_loading_log", _additional_file_loading_log->isChecked());

#ifdef NOGGIT_HAS_SCRIPTING
      _settings->setValue ("allow_scripts_write_any_file", _allow_scripts_write_any_file->isChecked());
#endif

#ifdef USE_MYSQL_UID_STORAGE
      _settings->setValue ("project/mysql/enabled", _mysql_box->isChecked());
      _settings->setValue ("project/mysql/server", _mysql_server_field->text());
      _settings->setValue ("project/mysql/user", _mysql_user_field->text());
      _settings->setValue ("project/mysql/pwd", _mysql_pwd_field->text());
      _settings->setValue ("project/mysql/db", _mysql_db_field->text());
#endif

      _settings->setValue ("wireframe/type", _wireframe_type_group->checkedId());
      _settings->setValue ("wireframe/radius", _wireframe_radius->value());
      _settings->setValue ("wireframe/width", _wireframe_width->value());
      _settings->setValue ("wireframe/color", _wireframe_color->color());      

	  _settings->sync();
    }
  }
}
