// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/SettingsPanel.h>

#include <noggit/settings.hpp>
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
                        NoggitSettings.set_value (key, value);
                      }
                    );
          }
        );


      browse_row (&gamePathField, "Game Path", "project/game_path", util::file_line_edit::directories);
      browse_row (&projectPathField, "Project Path", "project/path", util::file_line_edit::directories);
      browse_row (&importPathField, "Import Path", "project/import_file", util::file_line_edit::files);
      browse_row (&wmvLogPathField, "WMV Log Path", "project/wmv_log_file", util::file_line_edit::files);
      browse_row (&mclq_liquids_export_path, "MCLQ Liquids Export Path", "project/mclq_liquids_path", util::file_line_edit::directories);

      _mysql_box = new QGroupBox ("MySQL (uid storage)", this);
      _mysql_box->setToolTip ("Store the maps' max model unique id (uid) in a mysql database to sync your uids with different computers/users to avoid duplications");
      auto mysql_layout (new QFormLayout (_mysql_box));

#ifdef USE_MYSQL_UID_STORAGE
      _mysql_box->setCheckable (true);

      _mysql_server_field = new QLineEdit(NoggitSettings.value("project/mysql/server").toString(), this);
      _mysql_user_field = new QLineEdit(NoggitSettings.value("project/mysql/user").toString(), this);
      _mysql_pwd_field = new QLineEdit(NoggitSettings.value("project/mysql/pwd").toString(), this);
      _mysql_db_field = new QLineEdit(NoggitSettings.value("project/mysql/db").toString(), this);

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

      layout->addRow ( "FOV", _fov = new QDoubleSpinBox(this));
      _fov->setRange(10.f, 90.f);
      layout->addRow ( "View Distance"
                     , _view_distance = new QDoubleSpinBox
                     );
      _view_distance->setRange (0.f, 1048576.f);

      layout->addRow ( "Adt unloading distance (in adt)", _adt_unload_dist = new QSpinBox(this));
      _adt_unload_dist->setRange(1, 64);

      layout->addRow ("Adt unloading check interval (sec)", _adt_unload_check_interval = new QSpinBox(this));
      _adt_unload_check_interval->setMinimum(1);

      layout->addRow ("Adt loading radius", _adt_loading_radius = new QSpinBox(this));
      _adt_loading_radius->setMinimum(0);
      _adt_loading_radius->setMaximum(64);

      layout->addRow("Async loader thread count", _async_loader_thread_count = new QSpinBox(this));
      _async_loader_thread_count->setMinimum(1);
      _async_loader_thread_count->setMaximum(16);

      layout->addRow ("Always check for max UID", _uid_cb = new QCheckBox(this));

      layout->addRow ("Tablet support", tabletModeCheck = new QCheckBox(this));

      layout->addRow("Undock tool properties", _undock_tool_properties = new QCheckBox(this));
      layout->addRow("Undock quick access texture palette", _undock_small_texture_palette = new QCheckBox(this));

      layout->addRow("Additional file loading log", _additional_file_loading_log = new QCheckBox(this));
      layout->addRow("Use MCLQ Liquids (vanilla/BC) export", _use_mclq_liquids_export = new QCheckBox(this));
      layout->addRow("Only display one bounding box for models", _only_one_model_box = new QCheckBox(this));

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
      gamePathField->actual->setText (NoggitSettings.value ("project/game_path").toString());
      projectPathField->actual->setText (NoggitSettings.value ("project/path").toString());
      importPathField->actual->setText (NoggitSettings.value ("project/import_file").toString());
      wmvLogPathField->actual->setText (NoggitSettings.value ("project/wmv_log_file").toString());
      mclq_liquids_export_path->actual->setText (NoggitSettings.value ("project/mclq_liquids_path").toString());
      _fov->setValue (NoggitSettings.value ("fov", 54.f).toFloat());
      _view_distance->setValue (NoggitSettings.value ("view_distance", 1000.f).toFloat());
      tabletModeCheck->setChecked (NoggitSettings.value ("tablet/enabled", false).toBool());
      _undock_tool_properties->setChecked (NoggitSettings.value ("undock_tool_properties/enabled", true).toBool());
      _undock_small_texture_palette->setChecked (NoggitSettings.value ("undock_small_texture_palette/enabled", true).toBool());
      _vsync_cb->setChecked (NoggitSettings.value ("vsync", false).toBool());
      _anti_aliasing_cb->setChecked (NoggitSettings.value ("anti_aliasing", false).toBool());
      _fullscreen_cb->setChecked (NoggitSettings.value ("fullscreen", false).toBool());
      _adt_unload_dist->setValue(NoggitSettings.value("unload_dist", 5).toInt());
      _adt_unload_check_interval->setValue(NoggitSettings.value("unload_interval", 5).toInt());
      _adt_loading_radius->setValue(NoggitSettings.value("loading_radius", 1).toInt());
      _async_loader_thread_count->setValue(NoggitSettings.value("async_thread_count", 1).toInt());
      _uid_cb->setChecked(NoggitSettings.value("uid_startup_check", true).toBool());
      _additional_file_loading_log->setChecked(NoggitSettings.value("additional_file_loading_log", false).toBool());
      _use_mclq_liquids_export->setChecked(NoggitSettings.value("use_mclq_liquids_export", false).toBool());
      _only_one_model_box->setChecked(NoggitSettings.value("only_one_model_box", true).toBool());

#ifdef NOGGIT_HAS_SCRIPTING
      _allow_scripts_write_any_file->setChecked(NoggitSettings.value("allow_scripts_write_any_file",false).toBool());
#endif
#ifdef USE_MYSQL_UID_STORAGE
      _mysql_box->setChecked (NoggitSettings.value ("project/mysql/enabled").toBool());
      _mysql_server_field->setText (NoggitSettings.value ("project/mysql/server").toString());
      _mysql_user_field->setText(NoggitSettings.value ("project/mysql/user").toString());
      _mysql_pwd_field->setText (NoggitSettings.value ("project/mysql/pwd").toString());
      _mysql_db_field->setText (NoggitSettings.value ("project/mysql/db").toString());
#endif

      _wireframe_type_group->button (NoggitSettings.value ("wireframe/type", 0).toInt())->toggle();
      _wireframe_radius->setValue (NoggitSettings.value ("wireframe/radius", 1.5f).toFloat());
      _wireframe_width->setValue (NoggitSettings.value ("wireframe/width", 1.f).toFloat());
      _wireframe_color->setColor(NoggitSettings.value("wireframe/color").value<QColor>());
    }

    void settings::save_changes()
    {
      NoggitSettings.set_value ("project/game_path", gamePathField->actual->text());
      NoggitSettings.set_value ("project/path", projectPathField->actual->text());
      NoggitSettings.set_value ("project/import_file", importPathField->actual->text());
      NoggitSettings.set_value ("project/wmv_log_file", wmvLogPathField->actual->text());
      NoggitSettings.set_value ("project/mclq_liquids_path", mclq_liquids_export_path->actual->text());
      NoggitSettings.set_value ("fov", _fov->value());
      NoggitSettings.set_value ("view_distance", _view_distance->value());
      NoggitSettings.set_value ("tablet/enabled", tabletModeCheck->isChecked());
      NoggitSettings.set_value ("undock_tool_properties/enabled", _undock_tool_properties->isChecked());
      NoggitSettings.set_value ("undock_small_texture_palette/enabled", _undock_small_texture_palette->isChecked());
      NoggitSettings.set_value ("vsync", _vsync_cb->isChecked());
      NoggitSettings.set_value ("anti_aliasing", _anti_aliasing_cb->isChecked());
      NoggitSettings.set_value ("fullscreen", _fullscreen_cb->isChecked());
      NoggitSettings.set_value ("unload_dist", _adt_unload_dist->value());
      NoggitSettings.set_value ("unload_interval", _adt_unload_check_interval->value());
      NoggitSettings.set_value ("loading_radius", _adt_loading_radius->value());
      NoggitSettings.set_value ("async_thread_count", _async_loader_thread_count->value());
      NoggitSettings.set_value ("uid_startup_check", _uid_cb->isChecked());
      NoggitSettings.set_value ("additional_file_loading_log", _additional_file_loading_log->isChecked());
      NoggitSettings.set_value ("use_mclq_liquids_export", _use_mclq_liquids_export->isChecked());
      NoggitSettings.set_value ("only_one_model_box", _only_one_model_box->isChecked());

#ifdef NOGGIT_HAS_SCRIPTING
      NoggitSettings.set_value ("allow_scripts_write_any_file", _allow_scripts_write_any_file->isChecked());
#endif

#ifdef USE_MYSQL_UID_STORAGE
      NoggitSettings.set_value ("project/mysql/enabled", _mysql_box->isChecked());
      NoggitSettings.set_value ("project/mysql/server", _mysql_server_field->text());
      NoggitSettings.set_value ("project/mysql/user", _mysql_user_field->text());
      NoggitSettings.set_value ("project/mysql/pwd", _mysql_pwd_field->text());
      NoggitSettings.set_value ("project/mysql/db", _mysql_db_field->text());
#endif

      NoggitSettings.set_value ("wireframe/type", _wireframe_type_group->checkedId());
      NoggitSettings.set_value ("wireframe/radius", _wireframe_radius->value());
      NoggitSettings.set_value ("wireframe/width", _wireframe_width->value());
      NoggitSettings.set_value ("wireframe/color", _wireframe_color->color());

      NoggitSettings.sync();
    }
  }
}
