// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/SettingsPanel.h>

#include <algorithm>

#include <noggit/application.h> // fonts
#include <noggit/Settings.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/TextBox.h>
#include <noggit/ui/Texture.h>
#include <noggit/ui/MinimizeButton.h>
#include <noggit/Video.h> // video

#include "revision.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

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

UISettings::UISettings()
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
  connect ( viewDistanceField, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
          , [] (double val)
            {
              Settings::getInstance()->mapDrawDistance = val;
            }
          );

  layout->addRow ( "FarZ"
                 , farZField = new QDoubleSpinBox
                 );
  farZField->setRange (0.f, 1048576.f);
  connect ( farZField, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
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

  //! \todo make icon again
  layout->addRow ( new QLabel ("interface/gossipframe/availablequesticon.blp", this)
                 , new QLabel ("Changes may not take effect until next launch.", this)
                 );

  auto buttonBox ( new QDialogButtonBox ( QDialogButtonBox::Save
                                        | QDialogButtonBox::Cancel
                                        )
                 );

  layout->addWidget (buttonBox);

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

  //! \note the following are commented out since they might be in
  //! Settings but are things usually toggled with keybindings during
  //! editing and thus don't really belong here imho.

/*  addChild(autoselectCheck = new UICheckBox(95, 236, "Auto select mode", &Settings::getInstance()->AutoSelectingMode));

  layout->addWidget (new QLabel ("Model Tool", this));

    addChild(randRotCheck = new UICheckBox(358, 211, "Random rotation", &Settings::getInstance()->random_rotation));
    addChild(randSizeCheck = new UICheckBox(358, 236, "Random size", &Settings::getInstance()->random_size));
    addChild(randTiltCheck = new UICheckBox(358, 261, "Random tilt", &Settings::getInstance()->random_tilt));
    addChild(modelStatsCheck = new UICheckBox(358, 286, "Copy model stats", &Settings::getInstance()->copyModelStats));

    addChild(new UIText(22, height()-29, "Changes may not take effect until next launch.", app.getArial12(), eJustifyLeft));

    addChild(new UIButton(width()-110, height()-32, 100, 32, "Save", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", saveSettings, 0));
    addChild(new UIButton(width()-215, height()-32, 100, 32, "Cancel", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", discardSettingsChanges, 0));
*/
}

void UISettings::readInValues()
{
  gamePathField->actual->setText (QString::fromStdString (Settings::getInstance()->gamePath));
  projectPathField->actual->setText (QString::fromStdString (Settings::getInstance()->projectPath));
  wodPathField->actual->setText (QString::fromStdString (Settings::getInstance()->wodSavePath));
  importPathField->actual->setText (QString::fromStdString (Settings::getInstance()->importFile));
  wmvLogPathField->actual->setText (QString::fromStdString (Settings::getInstance()->wmvLogFile));
  viewDistanceField->setValue (Settings::getInstance()->mapDrawDistance);
  farZField->setValue (Settings::getInstance()->FarZ);
  tabletModeCheck->setChecked (Settings::getInstance()->tabletMode);
//     autoselectCheck->setState(Settings::getInstance()->AutoSelectingMode);
//     modelStatsCheck->setState(Settings::getInstance()->copyModelStats);
//     randRotCheck->setState(Settings::getInstance()->random_rotation);
//     randSizeCheck->setState(Settings::getInstance()->random_size);
//     randTiltCheck->setState(Settings::getInstance()->random_tilt);
}
