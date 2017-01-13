// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Environment.h"
#include "MapHeaders.h"

Environment::Environment()
{
	this->view_holelines = false;
	this->ShiftDown = false;
	this->AltDown = false;
	this->CtrlDown = false;
	this->SpaceDown = false;
	this->flagPaintMode = FLAG_IMPASS;
	this->paintMode = true;
  this->highlightPaintableChunks = true;
  this->flattenAngle = 0.0f;
  this->flattenOrientation = 0.0f;
  this->flattenAngleEnabled = false;
  this->minRotation = 0.0f;
  this->maxRotation = 360.0f;
  this->minTilt = -5.0f;
  this->maxTilt = 5.0f;
  this->minScale = 0.9f;
  this->maxScale = 1.1f;
  this->groundBrushType = 1;
  this->currentWaterLayer = 0;
  this->displayAllWaterLayers = true;
  this->moveModelToCursorPos = false;
  this->showModelFromHiddenList = true;
}

Environment* Environment::instance = 0;

Environment* Environment::getInstance()
{
	if (!instance)
		instance = new Environment();
	return instance;
}

selection_type Environment::get_clipboard()
{
	return *clipboard;
}

void Environment::set_clipboard(boost::optional<selection_type> set)
{
	clipboard = set;
}

void Environment::clear_clipboard()
{
  clipboard.reset();
}

bool Environment::is_clipboard()
{
	return !!clipboard;
}

math::vector_3d Environment::get_cursor_pos()
{
  return math::vector_3d(Pos3DX, Pos3DY, Pos3DZ);
}
