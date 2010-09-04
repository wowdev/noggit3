#include "Gui.h"

Gui::Gui()
{
	// Toolbar
	this->guiToolbar = new Toolbar( 6.0f, 38.0f, 105.0f, 600.0f, this );

	// Statusbar
	this->guiStatusbar = new statusBar( 0.0f, video.yres - 30.0f, video.xres, 30.0f );

	// DetailInfoWindow
	this->guidetailInfos = new detailInfos( 1.0f, video.yres - 282.0f, 530.0f, 250.0f, this );
	this->guidetailInfos->movable = true;
	this->guidetailInfos->hidden = true;

	// AppInfosWindow
	this->guiappInfo = new appInfo( 1.0f, video.yres - 440.0f, 420.0f, 410.0f, this );
	this->guiappInfo->movable = true;
	this->guiappInfo->hidden = true;
};
