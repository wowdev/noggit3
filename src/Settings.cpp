#include "Settings.h"

Settings::Settings()
{
	// set hardcoded till settings get serialized
	this->copy_rot=false;
	this->copy_size=false;
	this->copy_tile=false;
	this->AutoSelectingMode=true;
	this->holelinesOn=false;
}

Settings* Settings::instance = 0;

Settings* Settings::getInstance()
{
	if( !instance)
		instance = new Settings();
	return instance;
}

