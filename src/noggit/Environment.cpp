// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Environment.h>
#include <noggit/MapHeaders.h>

Environment::Environment()
  : groundBrushType(1)
  , minRotation(0.0f)
  , maxRotation(360.0f)
  , minTilt(-5.0f)
  , maxTilt(5.0f)
  , minScale(0.9f)
  , maxScale(1.1f)
  , moveModelToCursorPos(false)
  , displayAllWaterLayers(true)
  , currentWaterLayer(0)
{
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
