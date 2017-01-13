// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>
#include <map>

#include "Selection.h"
#include "math/vector_3d.hpp"

#include <boost/optional.hpp>

class Environment
{
public:
	static Environment* getInstance();
	selection_type get_clipboard();
	void set_clipboard(boost::optional<selection_type> entry);
	void clear_clipboard();
	bool is_clipboard();
  math::vector_3d get_cursor_pos();

	bool view_holelines;
	// values for areaID painting
	int selectedAreaID;
	std::map<int, math::vector_3d> areaIDColors; // List of all area IDs to draw them with different colors
	// hold keys
	bool ShiftDown;
	bool AltDown;
	bool CtrlDown;
	bool SpaceDown;

	bool paintMode;
  bool highlightPaintableChunks;
	int flagPaintMode;
	int screenX;
	int screenY;

	float Pos3DX;
	float Pos3DY;
	float Pos3DZ;

  bool flattenAngleEnabled;
  float flattenAngle;
  float flattenOrientation;

	float cursorColorR;
	float cursorColorG;
	float cursorColorB;
	float cursorColorA;
	int cursorType;

  int groundBrushType;

  float minRotation;
  float maxRotation;
  float minTilt;
  float maxTilt;
  float minScale;
  float maxScale;
  bool moveModelToCursorPos;

  bool displayAllWaterLayers;
  int currentWaterLayer;

  bool showModelFromHiddenList;

private:
	Environment();
	static Environment* instance;

  boost::optional<selection_type> clipboard;
};
