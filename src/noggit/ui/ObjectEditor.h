// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_3d.hpp>
#include <noggit/Selection.h>
#include <noggit/bool_toggle_property.hpp>

#include <QLabel>
#include <QWidget>

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
                    , object_paste_params*
                    );

      bool hasSelection() const;
      void copy(selection_type entry);
      void pasteObject ( math::vector_3d cursor_pos
                       , math::vector_3d camera_pos
                       , World*
                       , object_paste_params*
                       );
      void togglePasteMode();

      model_import *modelImport;
      rotation_editor* rotationEditor;
    private:
      QButtonGroup* pasteModeGroup;
      QLabel* _filename;

      bool _copy_model_stats;

      boost::optional<selection_type> selected;
      void showImportModels();
      void SaveObjecttoTXT (World*);
      void setModelName(const std::string &name);
      int pasteMode;
    };
  }
}
