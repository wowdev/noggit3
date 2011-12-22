#include "Environment.h"
#include "MapHeaders.h"

Environment::Environment()
{
  view_holelines = false;
  ShiftDown = false;
  AltDown = false;
  CtrlDown = false;
  clipboard = nameEntry();
  flagPaintMode = FLAG_IMPASS;
  paintMode = true;
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

