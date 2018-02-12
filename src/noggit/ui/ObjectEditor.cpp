// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/checkbox.hpp>
#include <util/qt/overload.hpp>

#include <QFormLayout>
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
    object_editor::object_editor ( MapView* mapView
                                 , World* world
                                 , bool_toggle_property* move_model_to_cursor_position
                                 , object_paste_params* paste_params
                                 )
            : QWidget(nullptr)
            , rotationEditor (new rotation_editor(mapView))
            , _settings (new QSettings (this))
            , _copy_model_stats (true)
            , selected()
            , pasteMode(PASTE_ON_TERRAIN)
    {
      auto layout = new QFormLayout (this);

      QGroupBox *copyBox = new QGroupBox("Copy options", this);
      auto copy_layout = new QFormLayout (copyBox);

      auto rotation_group (new QGroupBox ("Random rotation", copyBox));
      auto tilt_group (new QGroupBox ("Random tilt", copyBox));
      auto scale_group (new QGroupBox ("Random scale", copyBox));
      auto rotation_layout (new QFormLayout (rotation_group));
      auto tilt_layout (new QFormLayout (tilt_group));
      auto scale_layout (new QFormLayout (scale_group));

      rotation_group->setCheckable(true);
      rotation_group->setChecked(_settings->value ("model/random_rotation", false).toBool());
      tilt_group->setCheckable(true);
      tilt_group->setChecked(_settings->value ("model/random_tilt", false).toBool());
      scale_group->setCheckable(true);
      scale_group->setChecked(_settings->value ("model/random_size", false).toBool());

      QCheckBox *copyAttributesCheck = new QCheckBox("Copy rotation,\ntilt, and scale", this);

      QDoubleSpinBox *rotRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *rotRangeEnd = new QDoubleSpinBox(this);
      QDoubleSpinBox *tiltRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *tiltRangeEnd = new QDoubleSpinBox(this);
      QDoubleSpinBox *scaleRangeStart = new QDoubleSpinBox(this);
      QDoubleSpinBox *scaleRangeEnd = new QDoubleSpinBox(this);

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
      
      rotation_layout->addRow("Min:", rotRangeStart);
      rotation_layout->addRow("Max:", rotRangeEnd);
      copy_layout->addRow(rotation_group);

      tilt_layout->addRow("Min:", tiltRangeStart);
      tilt_layout->addRow("Max:", tiltRangeEnd);
      copy_layout->addRow(tilt_group);

      scale_layout->addRow("Min:", scaleRangeStart);
      scale_layout->addRow("Max:", scaleRangeEnd);
      copy_layout->addRow(scale_group);

      copy_layout->addRow(copyAttributesCheck);

      QGroupBox *pasteBox = new QGroupBox(this);
      auto paste_layout = new QFormLayout (pasteBox);
      QRadioButton *terrainButton = new QRadioButton("Terrain");
      QRadioButton *selectionButton = new QRadioButton("Selection");
      QRadioButton *cameraButton = new QRadioButton("Camera");

      pasteModeGroup = new QButtonGroup(this);
      pasteModeGroup->addButton(terrainButton, 0);
      pasteModeGroup->addButton(selectionButton, 1);
      pasteModeGroup->addButton(cameraButton, 2);

      auto cursorPosCheck ( new checkbox ( "Move model to\ncursor position"
                                         , move_model_to_cursor_position
                                         , this
                                         )
                          );

      pasteBox->setTitle("Paste Options");
      paste_layout->addRow(terrainButton);
      paste_layout->addRow(selectionButton);
      paste_layout->addRow(cameraButton);
      paste_layout->addRow(cursorPosCheck);

      QPushButton *rotEditorButton = new QPushButton("Pos/Rotation Editor", this);
      QPushButton *visToggleButton = new QPushButton("Toggle Visibility", this);
      QPushButton *clearListButton = new QPushButton("Clear List", this);

      QGroupBox *importBox = new QGroupBox(this);
      new QGridLayout (importBox);
      importBox->setTitle("Import");

      QPushButton *toTxt = new QPushButton("To Text File", this);
      QPushButton *fromTxt = new QPushButton("From Text File", this);
      importBox->layout()->addWidget(toTxt);
      importBox->layout()->addWidget(fromTxt);

      layout->addRow(copyBox);
      layout->addRow(pasteBox);
      layout->addRow(rotEditorButton);
      layout->addRow(visToggleButton);
      layout->addRow(clearListButton);
      layout->addRow(importBox);
      layout->addRow (_filename);

      connect (rotation_group, &QGroupBox::toggled, [&] (int s)
      {
        _settings->setValue ("model/random_rotation", s);
        _settings->sync();
      });

      connect (tilt_group, &QGroupBox::toggled, [&] (int s)
      {
        _settings->setValue ("model/random_tilt", s);
        _settings->sync();
      });

      connect (scale_group, &QGroupBox::toggled, [&] (int s)
      {
        _settings->setValue ("model/random_size", s);
        _settings->sync();
      });

      rotRangeStart->setValue(paste_params->minRotation);
      rotRangeEnd->setValue(paste_params->maxRotation);

      tiltRangeStart->setValue(paste_params->minTilt);
      tiltRangeEnd->setValue(paste_params->maxTilt);

      scaleRangeStart->setValue(paste_params->minScale);
      scaleRangeEnd->setValue(paste_params->maxScale);

      connect ( rotRangeStart, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->minRotation = v;
                }
      );

      connect ( rotRangeEnd, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->maxRotation = v;
                }
      );

      connect ( tiltRangeStart, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->minTilt = v;
                }
      );

      connect ( tiltRangeEnd, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->maxTilt = v;
                }
      );

      connect ( scaleRangeStart, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->minScale = v;
                }
      );

      connect ( scaleRangeEnd, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [=] (double v)
                {
                  paste_params->maxScale = v;
                }
      );

      copyAttributesCheck->setChecked(_copy_model_stats);
      connect (copyAttributesCheck, &QCheckBox::stateChanged, [this] (int s)
      {
        _copy_model_stats = s;
      });

      pasteModeGroup->button(pasteMode)->setChecked(true);

      connect ( pasteModeGroup, qOverload<int> (&QButtonGroup::buttonClicked)
              , [&] (int id)
                {
                    pasteMode = id;
                }
      );

      connect(rotEditorButton, &QPushButton::clicked, [=]() {
          rotationEditor->show();
      });

      connect(visToggleButton, &QPushButton::clicked, [=]() {
          mapView->_draw_hidden_models.set
            (!mapView->_draw_hidden_models.get());
      });

      connect(clearListButton, &QPushButton::clicked, [=]() {
        ModelManager::clear_hidden_models();
        WMOManager::clear_hidden_wmos();
      });

      connect(toTxt, &QPushButton::clicked, [=]() {
          SaveObjecttoTXT (world);
      });

      connect(fromTxt, &QPushButton::clicked, [=]() {
          showImportModels();
      });
    }

    void object_editor::showImportModels()
    {
      modelImport->show();
    }

    void object_editor::pasteObject ( math::vector_3d cursor_pos
                                    , math::vector_3d camera_pos
                                    , World* world
                                    , object_paste_params* paste_params
                                    )
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
          if (world->HasSelection())
          {
            auto selection = *world->GetCurrentSelection();
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

      {
        if (selected->which() == eEntry_Model)
        {
          float scale (1.f);
          math::vector_3d rotation (0.f, 0.f, 0.f);

          if (_copy_model_stats)
          {
            // copy rot size from original model. Dirty but woring
            scale = boost::get<selected_model_type> (selected.get())->scale;
            rotation = boost::get<selected_model_type> (selected.get())->dir;
          }

          world->addM2 ( boost::get<selected_model_type> (selected.get())->model->_filename
                       , pos
                       , scale
                       , rotation
                       , paste_params
                       );
        }
        else if (selected->which() == eEntry_WMO)
        {
          math::vector_3d rotation (0.f, 0.f, 0.f);
          if (_copy_model_stats)
          {
            // copy rot from original model. Dirty but working
            rotation = boost::get<selected_wmo_type> (selected.get())->dir;
          }

          world->addWMO(boost::get<selected_wmo_type> (selected.get())->wmo->_filename, pos, rotation);
        }
      }

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
        clone->scale = boost::get<selected_model_type> (entry)->scale;
        clone->dir = boost::get<selected_model_type> (entry)->dir;
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
        selected = boost::none;
        return;
      }
    }

    void object_editor::setModelName(const std::string &name)
    {
      std::stringstream ss;
      ss << "Model: " << name;
      _filename->setText(ss.str().c_str());
    }

    void object_editor::SaveObjecttoTXT (World* world)
    {
      if (!world->HasSelection())
        return;
      std::string path;

      if (world->IsSelection(eEntry_WMO))
      {
        path = boost::get<selected_wmo_type> (*world->GetCurrentSelection())->wmo->filename();
      }
      else if (world->IsSelection(eEntry_Model))
      {
        path = boost::get<selected_model_type> (*world->GetCurrentSelection())->model->_filename;
      }

      std::ofstream stream(_settings->value("project/import_file" , "import.txt").toString().toStdString(), std::ios_base::app);
      stream << path << std::endl;
      stream.close();

      modelImport->buildModelList();
    }
  }
}
