// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/SettingsPanel.h>

#include <noggit/Settings.h>
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
    {
      setWindowTitle ("Settings");

      auto layout (new QFormLayout (this));

      auto browse_row
        ( [&] (util::file_line_edit** line, char const* title, std::string* var, util::file_line_edit::mode mode)
          {
            layout->addRow
              ( title
              , *line = new util::file_line_edit (mode, title, this)
              );
            connect ( (*line)->actual, &QLineEdit::textChanged
                    , [var] (QString value)
                      {
                        *var = value.toStdString();
                      }
                    );
          }
        );

      browse_row (&gamePathField, "Game Path", &Settings::getInstance()->gamePath, util::file_line_edit::directories);
      browse_row (&projectPathField, "Project Path", &Settings::getInstance()->projectPath, util::file_line_edit::directories);
      browse_row (&wodPathField, "WoD Save Path", &Settings::getInstance()->wodSavePath, util::file_line_edit::directories);
      browse_row (&importPathField, "Import Path", &Settings::getInstance()->importFile, util::file_line_edit::files);
      browse_row (&wmvLogPathField, "WMV Log Path", &Settings::getInstance()->wmvLogFile, util::file_line_edit::files);

      layout->addRow ( "View Distance"
                     , viewDistanceField = new QDoubleSpinBox
                     );
      viewDistanceField->setRange (0.f, 1048576.f);
      connect ( viewDistanceField, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [] (double val)
                {
                  Settings::getInstance()->mapDrawDistance = val;
                }
              );

      layout->addRow ( "FarZ"
                     , farZField = new QDoubleSpinBox
                     );
      farZField->setRange (0.f, 1048576.f);
      connect ( farZField, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [] (double val)
                {
                  Settings::getInstance()->FarZ = val;
                }
              );

      layout->addRow ("Tablet support", tabletModeCheck = new QCheckBox ("enabled", this));
      connect ( tabletModeCheck, &QCheckBox::toggled
              , [] (bool enabled)
                {
                  Settings::getInstance()->tabletMode = enabled;
                }
              );

      auto warning (new QWidget (this));
      new QHBoxLayout (warning);
      auto icon (new QLabel (warning));
      icon->setPixmap
        (noggit::render_blp_to_pixmap ("interface/gossipframe/availablequesticon.blp"));
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
                  Settings::getInstance()->saveToDisk();
                }
              );

      connect ( buttonBox, &QDialogButtonBox::rejected
              , [this]
                {
                  hide();
                  Settings::getInstance()->readFromDisk();
                }
              );
    }

    void settings::readInValues()
    {
      gamePathField->actual->setText (QString::fromStdString (Settings::getInstance()->gamePath));
      projectPathField->actual->setText (QString::fromStdString (Settings::getInstance()->projectPath));
      wodPathField->actual->setText (QString::fromStdString (Settings::getInstance()->wodSavePath));
      importPathField->actual->setText (QString::fromStdString (Settings::getInstance()->importFile));
      wmvLogPathField->actual->setText (QString::fromStdString (Settings::getInstance()->wmvLogFile));
      viewDistanceField->setValue (Settings::getInstance()->mapDrawDistance);
      farZField->setValue (Settings::getInstance()->FarZ);
      tabletModeCheck->setChecked (Settings::getInstance()->tabletMode);
    }
  }
}
