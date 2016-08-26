#include "Environment.h"
#include "MapHeaders.h"

Environment::Environment()
{
	this->view_holelines = false;
	this->ShiftDown = false;
	this->AltDown = false;
	this->CtrlDown = false;
	this->SpaceDown = false;
	this->clipboard = nameEntry();
	this->flagPaintMode = FLAG_IMPASS;
	this->paintMode = true;
  this->flattenAngle = 0.0f;
  this->flattenOrientation = 0.0f;
  this->flattenAngleEnabled = false;
  this->minRotation = 0.0f;
  this->maxRotation = 360.0f;
  this->minTilt = -5.0f;
  this->maxTilt = 5.0f;
  this->minScale = 0.9f;
  this->maxScale = 1.1f;
}

Environment* Environment::instance = 0;

Environment* Environment::getInstance()
{
	if (!instance)
		instance = new Environment();
	return instance;
}

nameEntry Environment::get_clipboard()
{
	return clipboard;
}

void Environment::set_clipboard(nameEntry* set)
{
	clipboard = *set;
	clipboard_filled = true;
}

void Environment::clear_clipboard()
{
	clipboard_filled = false;
}

bool Environment::is_clipboard()
{
	return clipboard_filled;
}

Vec3D Environment::get_cursor_pos()
{
  return Vec3D(Pos3DX, Pos3DY, Pos3DZ);
}

