// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/settingsDialog.h>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>

#include <noggit/application.h>
#include <noggit/Log.h>

namespace noggit
{
  namespace ui
  {
    settingsDialog::settingsDialog()
    : QWidget()
    {
      const int minViewDistanceRange = 200;
      const int maxViewDistanceRange = 2048;

      currentGamePath = new QLabel(noggit::app().setting("paths/game").toString(), this);
      changeGamePathButton = new QPushButton("...", this);

      currentProjectPath = new QLabel(noggit::app().setting("paths/project").toString(), this);
      changeProjectPathButton = new QPushButton("...", this);

      addProjectPathCheckBox = new QCheckBox(tr("Select a different Project path"), this);

      viewDistanceSlider = new QSlider(Qt::Horizontal, this);
      viewDistanceSlider->setRange(minViewDistanceRange, maxViewDistanceRange);
      viewDistanceSlider->setValue(noggit::app().setting("view_distance").toReal());

      viewDistanceSpinBox = new QSpinBox();
      viewDistanceSpinBox->setRange(minViewDistanceRange, maxViewDistanceRange);
      viewDistanceSpinBox->setValue(viewDistanceSlider->value());

      antialiasingCheckBox = new QCheckBox(tr("Antialiasing"));
      if (noggit::app().setting("antialiasing").toBool())
        antialiasingCheckBox->setChecked(true);

      maximizedShowCheckBox = new QCheckBox(tr("Auto-Maximized Menu & Map Editor"));
      if (noggit::app().setting("maximizedShow").toBool())
        maximizedShowCheckBox->setChecked(true);

      maximizedAppShowCheckBox = new QCheckBox(tr("Maximized App on Start"));
      if (noggit::app().setting("maximizedAppShow").toBool())
        maximizedAppShowCheckBox->setChecked(true);

      projectExplorerShowCheckBox = new QCheckBox(tr("Open ProjectExplorer on Start"));
      if (noggit::app().setting("projectExplorerShow").toBool())
        projectExplorerShowCheckBox->setChecked(true);

      QLabel* _gamePathLabel (new QLabel(tr("Game path : "), this));
      projectPathLabel = new QLabel(tr("Project path : "), this);
      QLabel* _viewDistanceLabel (new QLabel(tr("View Distance : "), this));

      currentGamePath->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
      currentProjectPath->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
      changeGamePathButton->setFixedWidth(30);
      changeProjectPathButton->setFixedWidth(30);

      toggleGameProjectDisplay(currentGamePath->text() != currentProjectPath->text());

      QGridLayout* _mainConfigurationSettingsWindow (new QGridLayout);
      _mainConfigurationSettingsWindow->addWidget(_gamePathLabel, 0, 0);
      _mainConfigurationSettingsWindow->addWidget(currentGamePath, 0, 1, 1, 2);
      _mainConfigurationSettingsWindow->addWidget(changeGamePathButton, 0, 3);
      _mainConfigurationSettingsWindow->addWidget(addProjectPathCheckBox, 0, 4);
      _mainConfigurationSettingsWindow->addWidget(projectPathLabel, 1, 0);
      _mainConfigurationSettingsWindow->addWidget(currentProjectPath, 1, 1, 1, 2);
      _mainConfigurationSettingsWindow->addWidget(changeProjectPathButton, 1, 3);
      _mainConfigurationSettingsWindow->addWidget(antialiasingCheckBox, 2, 0);
      _mainConfigurationSettingsWindow->addWidget(maximizedShowCheckBox, 2, 1);
      _mainConfigurationSettingsWindow->addWidget(maximizedAppShowCheckBox, 3, 0);
      _mainConfigurationSettingsWindow->addWidget(projectExplorerShowCheckBox, 3, 1);
      _mainConfigurationSettingsWindow->addWidget(_viewDistanceLabel, 4, 0);
      _mainConfigurationSettingsWindow->addWidget(viewDistanceSlider, 4, 1);
      _mainConfigurationSettingsWindow->addWidget(viewDistanceSpinBox, 4, 2, 1, 2);

      _mainConfigurationSettingsWindow->setSpacing(10);

      setLayout(_mainConfigurationSettingsWindow);

      setWindowTitle(tr("configuration_settings"));
      setWindowIcon(QIcon("noggit.png"));

      viewDistanceSpinBox->connect(viewDistanceSlider, SIGNAL (valueChanged (int)), SLOT (setValue (int)));
      viewDistanceSlider->connect(viewDistanceSpinBox, SIGNAL (valueChanged (int)), SLOT (setValue (int)));

      connect(changeGamePathButton, SIGNAL (clicked()), this, SLOT (setGameAndProjectPath()));
      connect(changeProjectPathButton, SIGNAL (clicked()), this, SLOT (setProjectPath()));
      connect(addProjectPathCheckBox, SIGNAL (toggled (bool)), this, SLOT (toggleGameProjectDisplay (bool)));

      connect(antialiasingCheckBox, SIGNAL (toggled (bool)), SLOT (setAntialiasing (bool)));
      connect(maximizedShowCheckBox, SIGNAL (toggled (bool)), SLOT (setMaximizedShow (bool)));
      connect(maximizedAppShowCheckBox, SIGNAL (toggled (bool)), SLOT (setMaximizedAppShow (bool)));
      connect(projectExplorerShowCheckBox, SIGNAL (toggled (bool)), SLOT (setProjectExplorerShow (bool)));
      connect(viewDistanceSlider, SIGNAL (valueChanged (int)), SLOT (setViewDistance (int)));

      connect(&noggit::app(), SIGNAL (settingChanged (const QString&, const QVariant&)), SLOT (settingChanged (const QString&, const QVariant&)));
    }

    void settingsDialog::toggleGameProjectDisplay(bool checked)
    {
      addProjectPathCheckBox->setChecked(checked);
      currentProjectPath->setVisible(checked);
      changeProjectPathButton->setVisible(checked);
      projectPathLabel->setVisible(checked);

      if (checked == false)
      {
        currentProjectPath->setText(currentGamePath->text());
        noggit::app().set_setting ("paths/project", currentProjectPath->text());
      }
    }

    void settingsDialog::setGameAndProjectPath()
    {
      QString _path;

      _path = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

      if (_path != "")
      {
        _path.append("\\");
        currentGamePath->setText(_path);
        noggit::app().set_setting ("paths/game", currentGamePath->text());
        if (addProjectPathCheckBox->isChecked() == false)
        {
          currentProjectPath->setText(_path);
          noggit::app().set_setting ("paths/project", currentProjectPath->text());
        }
      }
    }

    void settingsDialog::setProjectPath()
    {
      QString _path;

      _path = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

      if (_path != "")
      {
        _path.append("/");
        currentProjectPath->setText(_path);
        noggit::app().set_setting ("paths/project", currentProjectPath->text());
      }
    }

    void settingsDialog::settingChanged(const QString& key, const QVariant& value)
    {
      if (key == "antialiasing")
        LogDebug << "antialiasing changed to : " << value.toString().toStdString() << std::endl;
      if (key == "maximizedShow")
        LogDebug << "maximizedShow changed to : " << value.toString().toStdString() << std::endl;
      if (key == "view_distance")
        LogDebug << "view distance changed to : " << value.toString().toStdString() << std::endl;
      if (key == "paths/game")
        LogDebug << "game path changed to : " << value.toString().toStdString() << std::endl;
      if (key == "paths/project")
        LogDebug << "project path changed to : " << value.toString().toStdString() << std::endl;
    }

    void settingsDialog::setAntialiasing(bool value)
    {
      noggit::app().set_setting ("antialiasing", value);
    }

    void settingsDialog::setMaximizedShow(bool value)
    {
      noggit::app().set_setting ("maximizedShow", value);
    }

    void settingsDialog::setMaximizedAppShow(bool value)
    {
      noggit::app().set_setting ("maximizedAppShow", value);
    }

    void settingsDialog::setProjectExplorerShow(bool value)
    {
      noggit::app().set_setting ("projectExplorerShow", value);
    }

    void settingsDialog::setViewDistance(int value)
    {
      noggit::app().set_setting ("view_distance", value);
    }
  }
}
