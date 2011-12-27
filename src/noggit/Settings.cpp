#include <noggit/Settings.h>

#include <boost/filesystem.hpp>

#include <noggit/Log.h>

#include <QSettings>

Settings::Settings()
  : _settings (new QSettings)
{
  // set hardcoded till settings get serialized
  copy_rot=false;
  copy_size=false;
  copy_tile=false;
  AutoSelectingMode=true;
  _noAntiAliasing = false;

  FarZ = _settings->value ("rendering/view_distance", 1024).toInt();
  _noAntiAliasing = !_settings->value ("rendering/antialiasing").toBool();
}

const bool& Settings::noAntiAliasing() const
{
  return _noAntiAliasing;
}

Settings* Settings::instance = 0;

Settings* Settings::getInstance()
{
  if( !instance)
    instance = new Settings();
  return instance;
}

