#include "Settings.h"

#include "ConfigFile.h"
#include "Directory.h" // FileExists
#include "Log.h"

Settings::Settings()
{
  // set hardcoded till settings get serialized
  this->copy_rot=false;
  this->copy_size=false;
  this->copy_tile=false;
  this->AutoSelectingMode=true;
  this->holelinesOn=false;
  this->FarZ = 1024;

  if( FileExists( "noggIt.conf" ) )
  {
    ConfigFile config( "noggIt.conf" );
    config.readInto( this->FarZ, "FarZ" );
  }

  if(this->FarZ != 1024 && this->FarZ != 2048 && this->FarZ != 3072 && this->FarZ != 4096)
  {
    LogError << "You set a wrong FarZ value in config file. Only 1024, 2048, 3072 and 4096 is allowed!" << std::endl;
    this->FarZ = 1024;
  }
}

Settings* Settings::instance = 0;

Settings* Settings::getInstance()
{
  if( !instance)
    instance = new Settings();
  return instance;
}

