// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/checkbox.hpp>
#include <util/qt/overload.hpp>

#include <boost/algorithm/string/predicate.hpp>

#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QtWidgets/QMessageBox>

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <sstream>

namespace noggit
{
  namespace ui
  {
    object_editor::object_editor ( MapView* mapView
                                 , World* world
                                 , bool_toggle_property* move_model_to_cursor_position
                                 , bool_toggle_property* snap_multi_selection_to_ground
                                 , bool_toggle_property* use_median_pivot_point
                                 , object_paste_params* paste_params
                                 , QWidget* parent
                                 )
            : QWidget(parent)
            , modelImport (new model_import(this))
            , rotationEditor (new rotation_editor(mapView, world))
            , helper_models_widget(new helper_models(this))
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
      auto rotation_layout (new QGridLayout (rotation_group));
      auto tilt_layout (new QGridLayout(tilt_group));
      auto scale_layout (new QGridLayout(scale_group));

      rotation_group->setCheckable(true);
      rotation_group->setChecked(_settings->value ("model/random_rotation", false).toBool());
      tilt_group->setCheckable(true);
      tilt_group->setChecked(_settings->value ("model/random_tilt", false).toBool());
      scale_group->setCheckable(true);
      scale_group->setChecked(_settings->value ("model/random_size", false).toBool());

      QCheckBox *copyAttributesCheck = new QCheckBox("Copy rotation, tilt, and scale", this);

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
      
      rotation_layout->addWidget(rotRangeStart, 0, 0);
      rotation_layout->addWidget(rotRangeEnd, 0 ,1);
      copy_layout->addRow(rotation_group);

      tilt_layout->addWidget(tiltRangeStart, 0, 0);
      tilt_layout->addWidget(tiltRangeEnd, 0, 1);
      copy_layout->addRow(tilt_group);

      scale_layout->addWidget(scaleRangeStart, 0, 0);
      scale_layout->addWidget(scaleRangeEnd, 0, 1);
      copy_layout->addRow(scale_group);

      copy_layout->addRow(copyAttributesCheck);

      QGroupBox *pasteBox = new QGroupBox("Paste Options", this);
      auto paste_layout = new QGridLayout (pasteBox);
      QRadioButton *terrainButton = new QRadioButton("Terrain");
      QRadioButton *selectionButton = new QRadioButton("Selection");
      QRadioButton *cameraButton = new QRadioButton("Camera");

      pasteModeGroup = new QButtonGroup(this);
      pasteModeGroup->addButton(terrainButton, 0);
      pasteModeGroup->addButton(selectionButton, 1);
      pasteModeGroup->addButton(cameraButton, 2);

      paste_layout->addWidget(terrainButton, 0, 0);
      paste_layout->addWidget(selectionButton, 0, 1);
      paste_layout->addWidget(cameraButton, 1, 0);

      auto object_movement_box (new QGroupBox("Single Selection Movement", this));
      auto object_movement_layout = new QFormLayout (object_movement_box);

      // single model selection
      auto object_movement_cb ( new checkbox ( "Mouse move follow\ncursor on the ground"
                                             , move_model_to_cursor_position
                                             , this
                                             )
                              );

      object_movement_layout->addRow(object_movement_cb);

      // multi model selection
      auto multi_select_movement_box(new QGroupBox("Multi Selection Movement", this));
      auto multi_select_movement_layout = new QFormLayout(multi_select_movement_box);

      auto multi_select_movement_cb ( new checkbox( "Mouse move snap\nmodels to the ground"
                                                  , snap_multi_selection_to_ground
                                                  , this
                                                  )
                                    );

      auto object_median_pivot_point (new checkbox ("Rotate around pivot point"
                                                   , use_median_pivot_point
                                                   , this
                                                   )
                                     );

      
      multi_select_movement_layout->addRow(multi_select_movement_cb);
      multi_select_movement_layout->addRow(object_median_pivot_point);

      QPushButton *rotEditorButton = new QPushButton("Pos/Rotation Editor", this);
      QPushButton *visToggleButton = new QPushButton("Toggle Hidden Models Visibility", this);
      QPushButton *clearListButton = new QPushButton("Clear Hidden Models List", this);

      QGroupBox *importBox = new QGroupBox(this);
      new QGridLayout (importBox);
      importBox->setTitle("Import");

      QPushButton *toTxt = new QPushButton("To Text File", this);
      QPushButton *fromTxt = new QPushButton("From Text File", this);
      QPushButton *last_m2_from_wmv = new QPushButton("Last M2 from WMV", this);
      QPushButton *last_wmo_from_wmv = new QPushButton("Last WMO from WMV", this);
      QPushButton *helper_models_btn = new QPushButton("Helper Models", this);
      importBox->layout()->addWidget(toTxt);
      importBox->layout()->addWidget(fromTxt);
      importBox->layout()->addWidget(last_m2_from_wmv);
      importBox->layout()->addWidget(last_wmo_from_wmv);
      importBox->layout()->addWidget(helper_models_btn);

      layout->addRow(copyBox);
      layout->addRow(pasteBox);
      layout->addRow(object_movement_box);
      layout->addRow(multi_select_movement_box);
      layout->addRow(rotEditorButton);
      layout->addRow(visToggleButton);
      layout->addRow(clearListButton);
      layout->addRow(importBox);
      layout->addRow (_filename);

      rotationEditor->use_median_pivot_point = &_use_median_pivot_point;

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

      connect (object_median_pivot_point, &QCheckBox::stateChanged, [this](bool b)
      {
          _use_median_pivot_point = b;
      });

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

      connect( last_m2_from_wmv
             , &QPushButton::clicked
             , [=]() { import_last_model_from_wmv(eEntry_Model); }
             );

      connect( last_wmo_from_wmv
             , &QPushButton::clicked
             , [=]() { import_last_model_from_wmv(eEntry_WMO); }
             );

      connect( helper_models_btn
             , &QPushButton::clicked
             , [=]() { helper_models_widget->show(); }
             );    

      setMinimumWidth(sizeHint().width());

      auto mv_pos = mapView->pos();
      auto mv_size = mapView->size();

      // make sure the window doesn't show up halfway outside the screen
      modelImport->move(mv_pos.x() + (mv_size.width() / 2), mv_pos.y() + (mv_size.height() / 2));
    }

    object_editor::~object_editor()
    {
      for (auto& instance : _model_instance_created)
      {
        if (instance.which() == eEntry_Model)
        {
          ModelInstance* mi = boost::get<selected_model_type>(instance);
          delete mi;
        }
        else if (instance.which() == eEntry_WMO)
        {
          WMOInstance* wi = boost::get<selected_wmo_type>(instance);
          delete wi;
        }
      }
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
      auto last_entry = world->get_last_selected_model();

      for (auto& selection : selected)
      {
        math::vector_3d pos;

        if (selection.which() == eEntry_MapChunk)
        {
          LogError << "Invalid selection" << std::endl;
          return;
        }

        math::vector_3d model_pos = selection.which() == eEntry_Model
          ? boost::get<selected_model_type>(selection)->pos
          : boost::get<selected_wmo_type>(selection)->pos
          ;

        switch (pasteMode)
        {
        case PASTE_ON_TERRAIN:
          pos = cursor_pos + model_pos;
          break;
        case PASTE_ON_SELECTION:
          if (last_entry)
          {
            math::vector_3d last_entry_pos = last_entry->which() == eEntry_Model
              ? boost::get<selected_model_type>(last_entry.get())->pos
              : boost::get<selected_wmo_type>(last_entry.get())->pos
              ;

            pos = last_entry_pos + model_pos;
          }
          else // paste to mouse cursor when there's no selected model
          {
            pos = cursor_pos + model_pos;
          }
          break;
        case PASTE_ON_CAMERA:
          pos = camera_pos + model_pos;
          break;
        default:
          LogDebug << "object_editor::pasteObject: unknown paste mode " << pasteMode << std::endl;
          break;
        }
        
        if (selection.which() == eEntry_Model)
        {
          float scale(1.f);
          math::vector_3d rotation(0.f, 0.f, 0.f);

          if (_copy_model_stats)
          {
            // copy rot size from original model. Dirty but woring
            scale = boost::get<selected_model_type>(selection)->scale;
            rotation = boost::get<selected_model_type>(selection)->dir;
          }

          world->addM2( boost::get<selected_model_type>(selection)->model->filename
                      , pos
                      , scale
                      , rotation
                      , paste_params
                      );
        }
        else if (selection.which() == eEntry_WMO)
        {
          math::vector_3d rotation(0.f, 0.f, 0.f);
          if (_copy_model_stats)
          {
            // copy rot from original model. Dirty but working
            rotation = boost::get<selected_wmo_type>(selection)->dir;
          }

          world->addWMO(boost::get<selected_wmo_type>(selection)->wmo->filename, pos, rotation);
        }        
      }
    }

    void object_editor::togglePasteMode()
    {
      pasteModeGroup->button ((pasteMode + 1) % PASTE_MODE_COUNT)->setChecked (true);
    }   

    void object_editor::replace_selection(std::vector<selection_type> new_selection)
    {
      selected = new_selection;

      std::stringstream ss;
      
      if (selected.empty())
      {
        _filename->setText("");
        return;
      }

      if (selected.size() == 1)
      {
        ss << "Model: ";

        auto selectedObject = new_selection.front();
        if (selectedObject.which() == eEntry_Model)
        {
          ss << boost::get<selected_model_type>(selectedObject)->model->filename;
        }
        else if (selectedObject.which() == eEntry_WMO)
        {
          ss << boost::get<selected_wmo_type>(selectedObject)->wmo->filename;
        }
        else
        {
          ss << "Error";
          LogError << "The new selection wasn't a m2 or wmo" << std::endl;
        }
      }
      else
      {
        ss << "Multiple objects selected";
      }

      _filename->setText(ss.str().c_str());
    }

    void object_editor::copy(std::string const& filename)
    {
      if (!MPQFile::exists(filename))
      {
        QMessageBox::warning
          ( nullptr
          , "Warning"
          , QString::fromStdString(filename + " not found.")
          );

        return;
      }

      std::vector<selection_type> selected_model;

      if (boost::ends_with (filename, ".m2"))
      {
        ModelInstance* mi = new ModelInstance(filename);

        _model_instance_created.push_back(mi);

        selected_model.push_back(mi);
        replace_selection(selected_model);
      }
      else if (boost::ends_with (filename, ".wmo"))
      {
        WMOInstance* wi = new WMOInstance(filename);

        _model_instance_created.push_back(wi);

        selected_model.push_back(wi);
        replace_selection(selected_model);
      }
    }

    void object_editor::copy_current_selection(World* world)
    {
      auto const& current_selection = world->current_selection();
      auto const& pivot = world->multi_select_pivot();

      if (current_selection.empty())
      {
        return;
      }

      std::vector<selection_type> selected_model;

      for (auto& selection : current_selection)
      {
        if (selection.which() == eEntry_Model)
        {
          auto original = boost::get<selected_model_type>(selection);
          auto clone = new ModelInstance(original->model->filename);
          
          clone->scale = original->scale;
          clone->dir = original->dir;
          clone->pos = pivot ? original->pos - pivot.get() : math::vector_3d();

          selected_model.push_back(clone);
          _model_instance_created.push_back(clone);
        }
        else if (selection.which() == eEntry_WMO)
        {
          auto original = boost::get<selected_wmo_type>(selection);
          auto clone = new WMOInstance(original->wmo->filename);
          clone->dir = original->dir;
          clone->pos = pivot ? original->pos - pivot.get() : math::vector_3d();

          selected_model.push_back(clone);
          _model_instance_created.push_back(clone);
        }
      }
      replace_selection(selected_model);
    }

    void object_editor::SaveObjecttoTXT (World* world)
    {
      if (!world->has_selection())
      {
        return;
      }

      std::ofstream stream(_settings->value("project/import_file", "import.txt").toString().toStdString(), std::ios_base::app);
      for (auto& selection : world->current_selection())
      {
        if (selection.which() == eEntry_MapChunk)
        {
          continue;
        }

        std::string path;

        if (selection.which() == eEntry_WMO)
        {
          path = boost::get<selected_wmo_type>(selection)->wmo->filename;
        }
        else if (selection.which() == eEntry_Model)
        {
          path = boost::get<selected_model_type>(selection)->model->filename;
        }

        stream << path << std::endl;
      }
      stream.close();
      modelImport->buildModelList();
    }

    void object_editor::import_last_model_from_wmv(int type)
    {
      std::string wmv_log_file (_settings->value ("project/wmv_log_file").toString().toStdString());
      std::string last_model_found;
      std::string line;
      std::ifstream file(wmv_log_file.c_str());

      if (file.is_open())
      {
        while (!file.eof())
        {
          getline(file, line);
          std::transform(line.begin(), line.end(), line.begin(), ::tolower);
          std::regex regex( type == eEntry_Model
                          ? "([a-z]+\\\\([a-z0-9_ ]+\\\\)*[a-z0-9_ ]+\\.)(mdx|m2)"
                          : "([a-z]+\\\\([a-z0-9_ ]+\\\\)*[a-z0-9_ ]+\\.)(wmo)"
                          );

          std::smatch match;

          if (std::regex_search (line, match, regex))
          {
            last_model_found = match.str(0);
            size_t found = last_model_found.rfind(".mdx");
            if (found != std::string::npos)
            {
              last_model_found.replace(found, 4, ".m2");
            }
          }
        }
      }
      else
      {
        QMessageBox::warning
          ( nullptr
          , "Warning"
          , "The wmv log file could not be opened"
          );
      }

      if(last_model_found == "")
      {
        QMessageBox::warning
          ( nullptr
          , "Warning"
          , "No corresponding model found in the wmv log file."
          );
      }
      else
      {
        copy(last_model_found);
      }      
    }

    QSize object_editor::sizeHint() const
    {
      return QSize(215, height());
    }
  }
}
