// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Selection.h>
#include <math/vector_3d.hpp>
#include <QLabel>
#include <QWidget>

class QButtonGroup;
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
  namespace ui
  {
    class object_editor : public QWidget
    {
    public:
      object_editor (MapView*);

      bool hasSelection() const;
      void copy(selection_type entry);
      void pasteObject (math::vector_3d cursor_pos, math::vector_3d camera_pos);
      void togglePasteMode();

      model_import *modelImport;
      rotation_editor* rotationEditor;
    private:
      QButtonGroup* pasteModeGroup;
      QLabel* _filename;

      boost::optional<selection_type> selected;
      void showImportModels();
      void SaveObjecttoTXT();
      void setModelName(const std::string &name);
      int pasteMode;
    };
  }
}
