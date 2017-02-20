// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>
#include <noggit/Selection.h>
#include <math/vector_3d.hpp>
#include <QDockWidget>

class UIModelImport;
class UIStatusBar;
class UIMapViewGUI;

enum ModelPasteMode
{
  PASTE_ON_TERRAIN,
  PASTE_ON_SELECTION,
  PASTE_ON_CAMERA,
  PASTE_MODE_COUNT
};

class UIObjectEditor : public QDockWidget
{
public:
  UIObjectEditor(float x, float y, UIMapViewGUI* mainGui);

  bool hasSelection() const;
  void copy(selection_type entry);
  void pasteObject (math::vector_3d cursor_pos);
  void togglePasteMode();

  UIModelImport *modelImport;
  UIStatusBar *filename;
private:
  UIToggleGroup *pasteModeGroup;
  UIMapViewGUI* mainGui;

  boost::optional<selection_type> selected;
  void toggleRotationEditor();
  void showImportModels();
  void SaveObjecttoTXT();
  void setModelName(const std::string &name);
  int pasteMode;
};
