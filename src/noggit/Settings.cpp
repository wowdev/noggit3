#include <noggit/Settings.h>

#include <boost/filesystem.hpp>

#include <noggit/ConfigFile.h>
#include <noggit/Log.h>

Settings::Settings()
{
  // set hardcoded till settings get serialized
  copy_rot=false;
  copy_size=false;
  copy_tile=false;
  AutoSelectingMode=true;
  holelinesOn=false;
  FarZ = 1024;
  _noAntiAliasing = false;

  if( boost::filesystem::exists( "noggIt.conf" ) )
  {
    ConfigFile config( "noggIt.conf" );
    config.readInto( FarZ, "FarZ" );
    _noAntiAliasing = config.readInto( _noAntiAliasing, "noAntiAliasing" );
  }
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

