// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>

#include <QLabel>
#include <QWidget>
#include <QSettings>

#include <boost/optional.hpp>

class MapView;
class QButtonGroup;
class World;

namespace noggit
{
  namespace ui
  {
    class model_import;
    class rotation_editor;
    class helper_models;
  }
}

enum ModelPasteMode
{
  PASTE_ON_TERRAIN,
  PASTE_ON_SELECTION,
  PASTE_ON_CAMERA,
  PASTE_MODE_COUNT
};

namespace noggit
{
  struct object_paste_params
  {
    float minRotation = 0.f;
    float maxRotation = 360.f;
    float minTilt = -5.f;
    float maxTilt = 5.f;
    float minScale = 0.9f;
    float maxScale = 1.1f;
  };

  namespace ui
  {
    class object_editor : public QWidget
    {
    public:
      object_editor ( MapView*
                    , World*
                    , bool_toggle_property* move_model_to_cursor_position
                    , bool_toggle_property* snap_multi_selection_to_ground
                    , bool_toggle_property* use_median_pivot_point
                    , object_paste_params*
                    , QWidget* parent = nullptr
                    );

      ~object_editor();

      void import_last_model_from_wmv(int type);
      void copy(std::string const& filename);
      void copy_current_selection(World* world);
      void pasteObject ( math::vector_3d cursor_pos
                       , math::vector_3d camera_pos
                       , World*
                       , object_paste_params*
                       );
      void togglePasteMode();

      model_import *modelImport;
      rotation_editor* rotationEditor;
      helper_models* helper_models_widget;
      QSize sizeHint() const override;

    private:
      QSettings* _settings;

      QButtonGroup* pasteModeGroup;
      QLabel* _filename;

      bool _copy_model_stats;
      bool _use_median_pivot_point;

      std::vector<selection_type> selected;
      std::vector<selection_type> _model_instance_created;
      
      void replace_selection(std::vector<selection_type> new_selection);

      void showImportModels();
      void SaveObjecttoTXT (World*);
      int pasteMode;
    };
  }
}
