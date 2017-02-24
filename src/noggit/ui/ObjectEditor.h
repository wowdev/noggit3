// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>
#include <noggit/Selection.h>
#include <math/vector_3d.hpp>
#include <QLabel>
#include <QWidget>

class UIModelImport;
class UIMapViewGUI;
class QButtonGroup;

enum ModelPasteMode
{
  PASTE_ON_TERRAIN,
  PASTE_ON_SELECTION,
  PASTE_ON_CAMERA,
  PASTE_MODE_COUNT
};

class UIObjectEditor : public QWidget
{
public:
  UIObjectEditor(float x, float y, UIMapViewGUI* mainGui);

  bool hasSelection() const;
  void copy(selection_type entry);
  void pasteObject (math::vector_3d cursor_pos, math::vector_3d camera_pos);
  void togglePasteMode();

  UIModelImport *modelImport;
private:
  QButtonGroup* pasteModeGroup;
  UIMapViewGUI* mainGui;
  QLabel* _filename;

  boost::optional<selection_type> selected;
  void toggleRotationEditor();
  void showImportModels();
  void SaveObjecttoTXT();
  void setModelName(const std::string &name);
  int pasteMode;
};
