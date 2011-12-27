#include <noggit/Environment.h>
#include <noggit/MapHeaders.h>

#include <QSettings>

Environment::Environment()
  : _settings (new QSettings)
{
  ShiftDown = false;
  AltDown = false;
  CtrlDown = false;
  clipboard = nameEntry();
  flagPaintMode = FLAG_IMPASS;
  paintMode = true;

  cursorColorR = _settings->value ("cursor/red", 1.0f).toFloat();
  cursorColorG = _settings->value ("cursor/green", 1.0f).toFloat();
  cursorColorB = _settings->value ("cursor/blue", 1.0f).toFloat();
  cursorColorA = _settings->value ("cursor/alpha", 1.0f).toFloat();
  cursorType = _settings->value ("cursor/type", 1.0f).toInt();
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

