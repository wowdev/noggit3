#include "Gui.h"

#include "Toolbar.h" // Toolbar
#include "statusBar.h" // statusBar
#include "detailInfos.h" // detailInfos
#include "appInfo.h" // appInfo
#include "ui_ZoneIdBrowser.h" // appInfo
#include "video.h" // video
#include "MapView.h"
#include "minimapWindowUI.h"
#include "world.h"

Gui::Gui(MapView *setMapview)
{
	this->theMapview = setMapview;
	this->tileFrames = new frame( 0.0f, 0.0f, video.xres, video.yres );
	
	// Minimap window
	this->minimapWindow = new minimapWindowUI(gWorld);
	this->minimapWindow->hidden = true;
	this->tileFrames->addChild(this->minimapWindow);

	// Toolbar
	this->guiToolbar = new Toolbar( 6.0f, 38.0f, 105.0f, 600.0f, this );
	this->tileFrames->addChild(this->guiToolbar);
	
	// Statusbar
	this->guiStatusbar =	new statusBar( 0.0f, video.yres - 30.0f, video.xres, 30.0f );
	this->tileFrames->addChild(this->guiStatusbar);

	// DetailInfoWindow
	this->guidetailInfos = new detailInfos( 1.0f, video.yres - 282.0f, 530.0f, 250.0f, this );
	this->guidetailInfos->movable = true;
	this->guidetailInfos->hidden = true;
	this->tileFrames->addChild(this->guidetailInfos);

	// ZoneIDBrowser
	this->ZoneIDBrowser = new ui_ZoneIdBrowser(200, 200, 600, 400, this);
	this->ZoneIDBrowser->movable = true;
	this->ZoneIDBrowser->hidden = true;
	this->tileFrames->addChild(this->ZoneIDBrowser);

	// AppInfosWindow
	this->guiappInfo = new appInfo( 1.0f, video.yres - 440.0f, 420.0f, 410.0f, this );
	this->guiappInfo->movable = true;
	this->guiappInfo->hidden = true;
	this->tileFrames->addChild(this->guiappInfo);
}