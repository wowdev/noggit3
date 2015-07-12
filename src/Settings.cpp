#include "Settings.h"

#include <boost/filesystem.hpp>

#include "ConfigFile.h"
#include "Log.h"

Settings::Settings()
{
	// set hardcoded till settings get serialized
	this->copy_rot = false;
	this->copy_size = false;
	this->copy_tile = false;
	this->AutoSelectingMode = true;
	this->holelinesOn = false;
	this->FarZ = 1024;
	this->_noAntiAliasing = false;
	this->copyModelStats = true;

	if (boost::filesystem::exists("noggit.conf"))
	{
		ConfigFile config("noggit.conf");
		config.readInto(this->FarZ, "FarZ");
		_noAntiAliasing = config.readInto(_noAntiAliasing, "noAntiAliasing");
	}
}

const bool& Settings::noAntiAliasing() const
{
	return _noAntiAliasing;
}

Settings* Settings::instance = 0;

Settings* Settings::getInstance()
{
	if (!instance)
		instance = new Settings();
	return instance;
}

