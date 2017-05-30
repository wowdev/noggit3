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
    settings::settings()
      : QDialog (nullptr)
      , _settings (new QSettings (this))
    {
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
      browse_row (&wodPathField, "WoD Save Path", "project/wod_save_path", util::file_line_edit::directories);
      browse_row (&importPathField, "Import Path", "project/import_file", util::file_line_edit::files);
      browse_row (&wmvLogPathField, "WMV Log Path", "project/wod_save_path", util::file_line_edit::files);

      layout->addRow ( "View Distance"
                     , viewDistanceField = new QDoubleSpinBox
                     );
      viewDistanceField->setRange (0.f, 1048576.f);      

      layout->addRow ( "FarZ"
                     , farZField = new QDoubleSpinBox
                     );
      farZField->setRange (0.f, 1048576.f);


      layout->addRow ("Tablet support", tabletModeCheck = new QCheckBox ("enabled", this));


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
    }

    void settings::discard_changes()
    {
      gamePathField->actual->setText (_settings->value ("project/game_path").toString());
      projectPathField->actual->setText (_settings->value ("project/path").toString ());
      wodPathField->actual->setText (_settings->value ("project/wod_save_path").toString ());
      importPathField->actual->setText (_settings->value ("project/import_file").toString ());
      wmvLogPathField->actual->setText (_settings->value ("project/wmv_log_file").toString ());
      viewDistanceField->setValue (_settings->value ("view_distance", 500.f).toFloat());
      farZField->setValue (_settings->value ("farZ", 2048.f).toFloat());
      tabletModeCheck->setChecked (_settings->value ("tablet/enabled", false).toBool());
    }

    void settings::save_changes()
    {
      _settings->setValue ("project/game_path", gamePathField->actual->text());
      _settings->setValue ("project/path", projectPathField->actual->text());
      _settings->setValue ("project/wod_save_path", wodPathField->actual->text());
      _settings->setValue ("project/import_file", importPathField->actual->text());
      _settings->setValue ("project/wmv_log_file", wmvLogPathField->actual->text());
      _settings->setValue ("farZ", viewDistanceField->value());
      _settings->setValue ("view_distance", farZField->value());
      _settings->setValue ("tablet/enabled", tabletModeCheck->isChecked());
    }
  }
}
