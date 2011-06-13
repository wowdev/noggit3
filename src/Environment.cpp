#include "Environment.h"
#include "MapHeaders.h"

Environment::Environment()
{
  this->view_holelines = false;
  this->ShiftDown = false;
  this->AltDown = false;
  this->CtrlDown = false;
  this->AutoSelecting = true;
  this->clipboard = nameEntry();
  this->flagPaintMode = FLAG_IMPASS;
  this->paintMode = true;
}

Environment* Environment::instance = 0;

Environment* Environment::getInstance()
{
  if( !instance )
    instance = new Environment();
  return instance;
}

nameEntry Environment::get_clipboard()
{
  return clipboard;
}

void Environment::set_clipboard( nameEntry* set )
{
  clipboard = *set;
}

