// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <noggit/Environment.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/Settings.h>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/application.h> // fonts
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>

#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>


namespace noggit
{
  namespace ui
  {
    object_editor::object_editor (MapView* mapView)
            : QWidget(nullptr)
      , rotationEditor (new rotation_editor())
            , selected()
            , pasteMode(PASTE_ON_TERRAIN)
    {
      auto inspectorGrid = new QGridLayout (this);

      Environment* env = Environment::getInstance();

      QGroupBox *copyBox = new QGroupBox(this);
      auto copyGrid = new QGridLayout (copyBox);
      QCheckBox *randRotCheck = new QCheckBox("Random rotation", this);
      QCheckBox *randTiltCheck = new QCheckBox("Random tilt", this);
      QCheckBox *randScaleCheck = new QCheckBox("Random scale", this);
      QCheckBox *copyAttributesCheck = new QCheckBox("Copy rotation, tilt, and scale", this);

      QDoubleSpinBox *rotRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *rotRangeEnd = new QDoubleSpinBox(this);
      QDoubleSpinBox *tiltRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *tiltRangeEnd = new QDoubleSpinBox(this);
      QDoubleSpinBox *scaleRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *scaleRangeEnd = new QDoubleSpinBox(this);

      QLabel *minLabel = new QLabel("Min", this);
      QLabel *maxLabel = new QLabel("Max", this);

      _filename = new QLabel (this);
      _filename->setWordWrap (true);

      rotRangeStart->setMaximumWidth(85);
      rotRangeEnd->setMaximumWidth(85);
      tiltRangeStart->setMaximumWidth(85);
      tiltRangeEnd->setMaximumWidth(85);
      scaleRangeStart->setMaximumWidth(85);
      scaleRangeEnd->setMaximumWidth(85);

      rotRangeStart->setDecimals(3);
      rotRangeEnd->setDecimals(3);
      tiltRangeStart->setDecimals(3);
      tiltRangeEnd->setDecimals(3);
      scaleRangeStart->setDecimals(3);
      scaleRangeEnd->setDecimals(3);

      rotRangeStart->setRange (-180.f, 180.f);
      rotRangeEnd->setRange (-180.f, 180.f);
      tiltRangeStart->setRange (-180.f, 180.f);
      tiltRangeEnd->setRange (-180.f, 180.f);
      scaleRangeStart->setRange (-180.f, 180.f);
      scaleRangeEnd->setRange (-180.f, 180.f);

      copyBox->setTitle("Copy Options");
      copyGrid->addWidget(minLabel, 0, 1, 1, 1, Qt::AlignCenter);
      copyGrid->addWidget(maxLabel, 0, 2, 1, 1, Qt::AlignCenter);

      copyGrid->addWidget(randRotCheck, 1, 0, 1, 1);
      copyGrid->addWidget(rotRangeStart, 1, 1, 1, 1);
      copyGrid->addWidget(rotRangeEnd, 1, 2, 1, 1);
      copyGrid->addWidget(randTiltCheck, 3, 0, 1, 1);
      copyGrid->addWidget(tiltRangeStart, 3, 1, 1, 1);
      copyGrid->addWidget(tiltRangeEnd, 3, 2, 1, 1);
      copyGrid->addWidget(randScaleCheck, 4, 0, 1, 1);
      copyGrid->addWidget(scaleRangeStart, 4, 1, 1, 1);
      copyGrid->addWidget(scaleRangeEnd, 4, 2, 1, 1);
      copyGrid->addWidget(copyAttributesCheck, 5, 0, 1, 3);

      QGroupBox *pasteBox = new QGroupBox(this);
      auto pasteGrid = new QGridLayout (pasteBox);
      QRadioButton *terrainButton = new QRadioButton("Terrain");
      QRadioButton *selectionButton = new QRadioButton("Selection");
      QRadioButton *cameraButton = new QRadioButton("Camera");

      pasteModeGroup = new QButtonGroup(this);
      pasteModeGroup->addButton(terrainButton, 0);
      pasteModeGroup->addButton(selectionButton, 1);
      pasteModeGroup->addButton(cameraButton, 2);

      QCheckBox *cursorPosCheck = new QCheckBox("Move model to cursor position", this);

      pasteBox->setTitle("Paste Options");
      pasteGrid->addWidget(terrainButton);
      pasteGrid->addWidget(selectionButton, 0, 1, 1, 1);
      pasteGrid->addWidget(cameraButton, 0, 2, 1, 1);
      pasteGrid->addWidget(cursorPosCheck, 1, 0, 1, 3);

      QPushButton *rotEditorButton = new QPushButton("Rotation Editor", this);
      QPushButton *visToggleButton = new QPushButton("Toggle Visibility", this);
      QPushButton *clearListButton = new QPushButton("Clear List", this);

      QGroupBox *importBox = new QGroupBox(this);
      new QGridLayout (importBox);
      importBox->setTitle("Import");

      QPushButton *toTxt = new QPushButton("To Text File", this);
      QPushButton *fromTxt = new QPushButton("From Text File", this);
      importBox->layout()->addWidget(toTxt);
      importBox->layout()->addWidget(fromTxt);

      inspectorGrid->addWidget(copyBox, 0, 0, 1, 2);
      inspectorGrid->addWidget(pasteBox, 1, 0, 1, 2);
      inspectorGrid->addWidget(rotEditorButton, 2, 0, 1, 1);
      inspectorGrid->addWidget(visToggleButton, 3, 0, 1, 1);
      inspectorGrid->addWidget(clearListButton, 4, 0, 1, 1);
      inspectorGrid->addWidget(importBox, 2, 1, 3, 1);
      inspectorGrid->addWidget (_filename, 5, 0, 1, 2);

    //    setWidget(content);

      randRotCheck->setChecked(Settings::getInstance()->random_rotation);
      connect (randRotCheck, &QCheckBox::stateChanged, [] (int s)
      {
        Settings::getInstance()->random_rotation = s;
      });

      rotRangeStart->setValue(env->minRotation);
      rotRangeEnd->setValue(env->maxRotation);

      tiltRangeStart->setValue(env->minTilt);
      tiltRangeEnd->setValue(env->maxTilt);

      scaleRangeStart->setValue(env->minScale);
      scaleRangeEnd->setValue(env->maxScale);

      connect ( rotRangeStart, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                    env->minRotation = v;
                }
      );

      connect ( rotRangeEnd, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                    env->maxRotation = v;
                }
      );

      connect ( tiltRangeStart, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                    env->minTilt = v;
                }
      );

      connect ( tiltRangeEnd, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                    env->maxTilt = v;
                }
      );

      connect ( scaleRangeStart, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                    env->minScale = v;
                }
      );

      connect ( scaleRangeEnd, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                    env->maxScale = v;
                }
      );

      randTiltCheck->setChecked(Settings::getInstance()->random_tilt);
      connect (randTiltCheck, &QCheckBox::stateChanged, [] (int s)
      {
        Settings::getInstance()->random_tilt = s;
      });

      randScaleCheck->setChecked(Settings::getInstance()->random_tilt);
      connect (randScaleCheck, &QCheckBox::stateChanged, [] (int s)
      {
        Settings::getInstance()->random_size = s;
      });

      copyAttributesCheck->setChecked(Settings::getInstance()->copyModelStats);
      connect (copyAttributesCheck, &QCheckBox::stateChanged, [] (int s)
      {
        Settings::getInstance()->copyModelStats = s;
      });

      pasteModeGroup->button(pasteMode)->setChecked(true);

      connect ( pasteModeGroup, static_cast<void (QButtonGroup::*) (int)> (&QButtonGroup::buttonClicked)
              , [&] (int id)
                {
                    pasteMode = id;
                }
      );

      cursorPosCheck->setChecked(Environment::getInstance()->moveModelToCursorPos);
      connect (cursorPosCheck, &QCheckBox::stateChanged, [=] (int s)
      {
        env->moveModelToCursorPos = s;
      });

      connect(rotEditorButton, &QPushButton::clicked, [=]() {
          rotationEditor->show();
      });

      connect(visToggleButton, &QPushButton::clicked, [=]() {
          mapView->_draw_hidden_models.set
            (!mapView->_draw_hidden_models.get());
      });

      connect(clearListButton, &QPushButton::clicked, [=]() {
          mapView->_hidden_map_objects.clear();
          mapView->_hidden_models.clear();
      });

      connect(toTxt, &QPushButton::clicked, [=]() {
          SaveObjecttoTXT();
      });

      connect(fromTxt, &QPushButton::clicked, [=]() {
          showImportModels();
      });
    }

    void object_editor::showImportModels()
    {
      modelImport->show();
    }

    void object_editor::pasteObject (math::vector_3d cursor_pos, math::vector_3d camera_pos)
    {
      if (!hasSelection() || selected->which() == eEntry_MapChunk)
      {
        return;
      }

      math::vector_3d pos = cursor_pos;

      switch (pasteMode)
      {
        case PASTE_ON_TERRAIN: // use cursor pos
          break;
        case PASTE_ON_SELECTION:
          if (gWorld->HasSelection())
          {
            auto selection = *gWorld->GetCurrentSelection();
            if (selection.which() == eEntry_Model)
            {
              pos = boost::get<selected_model_type> (selection)->pos;
            }
            else if (selection.which() == eEntry_WMO)
            {
              pos = boost::get<selected_wmo_type> (selection)->pos;
            }
          } // else: use cursor pos
          break;
        case PASTE_ON_CAMERA:
          pos = camera_pos;
          break;
        default:
          LogDebug << "UIObjectEditor::pasteObject: Unknown pasteMode " << pasteMode << std::endl;
          break;
      }

      gWorld->addModel(selected.get(), pos, true);
    }

    void object_editor::togglePasteMode()
    {
      pasteModeGroup->button ((pasteMode + 1) % PASTE_MODE_COUNT)->setChecked (true);
    }

    bool object_editor::hasSelection() const
    {
      return !!selected;
    }

    void object_editor::copy(selection_type entry)
    {
      if (entry.which() == eEntry_Model)
      {
        auto clone = new ModelInstance(boost::get<selected_model_type> (entry)->model->_filename);
        clone->sc = boost::get<selected_model_type> (entry)->sc;
        clone->dir = boost::get<selected_model_type> (entry)->dir;
        clone->ldir = boost::get<selected_model_type> (entry)->ldir;
        selected = clone;
        setModelName (boost::get<selected_model_type> (entry)->model->_filename);
      }
      else if (entry.which() == eEntry_WMO)
      {
        auto clone = new WMOInstance(boost::get<selected_wmo_type> (entry)->wmo->_filename);
        clone->dir = boost::get<selected_wmo_type> (entry)->dir;
        selected = clone;
        setModelName(boost::get<selected_wmo_type> (entry)->wmo->_filename);
      }
      else
      {
        Environment::getInstance()->clear_clipboard();
        return;
      }

      Environment::getInstance()->set_clipboard(selected);
    }

    void object_editor::setModelName(const std::string &name)
    {
      std::stringstream ss;
      ss << "Model: " << name;
      _filename->setText(ss.str().c_str());
    }

    void object_editor::SaveObjecttoTXT()
    {
      if (!gWorld->HasSelection())
        return;
      std::string path;

      if (gWorld->IsSelection(eEntry_WMO))
      {
        path = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->wmo->filename();
      }
      else if (gWorld->IsSelection(eEntry_Model))
      {
        path = boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->model->_filename;
      }

      std::ofstream stream(Settings::getInstance()->importFile, std::ios_base::app);
      stream << path << std::endl;
      stream.close();

      modelImport->buildModelList();
    }
  }
}
