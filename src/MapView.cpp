#include <cmath>
#include <string>
#include <map>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "MapView.h"
//#include "trace.h"
#include "noggit.h" // gStates, gPop, gFPS, arial14, morpheus40, arial...
#include "Log.h"
#include "dbc.h"
#include "texturingui.h"
#include "world.h"
#include "MapChunk.h"
#include "ConfigFile.h"
#include "appInfo.h" // appInfo

#include "FreeType.h" // freetype::

#include "Settings.h"
#include "Project.h"
#include "Environment.h"

#include "brush.h" // brush

#include "WMOInstance.h" // WMOInstance
#include "TextureManager.h" // TextureManager, Texture
#include "ModelManager.h" // ModelManager

#include "directory.h" // FileExists

// ui classes
#include "slider.h" // slider
#include "Gui.h" // Gui
#include "ToggleGroup.h" // ToggleGroup
#include "textUI.h" // textUI
#include "gradient.h" // gradient
#include "checkboxUI.h" // checkboxUI
#include "Toolbar.h" // Toolbar
#include "Icon.h" // Icon
#include "textureUI.h" // textureUI
#include "statusBar.h" // statusBar
#include "detailInfos.h" // detailInfos
#include "menuBar.h" // menuBar, menu items, ..


static const float XSENS = 15.0f;
static const float YSENS = 15.0f;
static const float SPEED = 66.6f;

int MouseX;
int MouseY;
float mh,mv,rh,rv;

float moveratio = 0.1f;
float rotratio = 0.2f;
float keyx,keyy,keyz,keyr,keys;

int tool_settings_x;
int tool_settings_y;

bool MoveObj;

Vec3D ObjMove;
Vec3D ObjRot;


bool TestSelection=false;

extern bool DrawMapContour;
extern bool drawFlags;

extern nameEntryManager SelectionNames;

// extern row and col form Palette UI


// This variables store the current status of the
// Shift, Alt and CTRL keys


bool	leftMouse=false;			
bool	leftClicked=false;
bool	rightMouse=false;
bool	painting=false;

// Vars for the ground editing toggle mode
// store the status of some view settings when
// the ground editing mode is switched on
// to restore them if switch back again

bool	alloff = true;
bool	alloff_models = false;
bool	alloff_doodads = false;
bool	alloff_contour = false;
bool	alloff_wmo = false;
bool	alloff_detailselect = false;
bool	alloff_fog = false;
bool	alloff_terrain = false;

slider *ground_brush_radius;
float groundBrushRadius=15.0f;
slider *ground_brush_speed;
float groundBrushSpeed=1.0f;
int		groundBrushType=2;

slider *blur_brush;
float blurBrushRadius=10.0f;
int		blurBrushType=2;

slider *paint_brush;

float brushPressure=0.9f;
float brushLevel=255.0f;

int terrainMode=0;

brush textureBrush;

bool Saving=false;

frame *LastClicked;


// main GUI object
Gui *mainGui;

frame	*MapChunkWindow;
frame	*TexturePalette;
frame	*SelectedTexture;
frame	*fakeframe;

ToggleGroup * gBlurToggleGroup;
ToggleGroup * gGroundToggleGroup;

window *setting_ground;
window *setting_blur;
window *settings_paint;


//TextBox * textbox;

void setGroundBrushRadius(float f)
{
	groundBrushRadius = f;
}
void setGroundBrushSpeed(float f)
{
	groundBrushSpeed =f;
}

void setBlurBrushRadius(float f)
{
	blurBrushRadius = f;
}


void setTextureBrushHardness(float f)
{
	textureBrush.setHardness(f);
}

void setTextureBrushRadius(float f)
{
	textureBrush.setRadius(f);
}

void setTextureBrushPressure(float f)
{
	brushPressure=f;
}

void setTextureBrushLevel(float f)
{
	brushLevel=(1.0f-f)*255.0f;
}

void SaveOrReload( frame*, int pMode )
{
	if( pMode )
		gWorld->reloadTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );
	else
		gWorld->saveTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );
}

void change_settings_window(int oldid, int newid)
{
	setting_ground->hidden=true;
	setting_blur->hidden=true;
	settings_paint->hidden=true;

	// fetch old win position
	switch(oldid)
	{
	case 1:
		tool_settings_x=setting_ground->x;
		tool_settings_y=setting_ground->y;
	break;
	case 2:
		tool_settings_x=setting_blur->x;
		tool_settings_y=setting_blur->y;
	break;
	case 3:
		tool_settings_x=settings_paint->x;
		tool_settings_y=settings_paint->y;
	break;
	}
	// set new win pos and make visible
	switch(newid)
	{
	case 1:
		setting_ground->x=tool_settings_x;
		setting_ground->y=tool_settings_y;
		setting_ground->hidden=false;
	break;
	case 2:
		setting_blur->x=tool_settings_x;
		setting_blur->y=tool_settings_y;
		setting_blur->hidden=false;
	break;
	case 3:
		settings_paint->x=tool_settings_x;
		settings_paint->y=tool_settings_y;
		settings_paint->hidden=false;
	break;
	}

}

//! \todo	Do this nicer?
void openHelp( frame *button, int id )
{
	reinterpret_cast<MapView*>( gStates.back() )->ViewHelp();
}

void closeHelp( frame *button, int id )
{
	reinterpret_cast<MapView*>( gStates.back() )->View3D();
}

void ResetSelectedObjectRotation( frame *button, int id )
{
	if( gWorld->IsSelection( eEntry_WMO ) )
		gWorld->GetCurrentSelection()->data.wmo->resetDirection();
	else if( gWorld->IsSelection( eEntry_Model ) )
		gWorld->GetCurrentSelection()->data.model->resetDirection();
}

void SnapSelectedObjectToGround( frame *button, int id )
{
	if( gWorld->IsSelection( eEntry_WMO ) )
	{
		Vec3D t = Vec3D( gWorld->GetCurrentSelection()->data.wmo->pos.x, gWorld->GetCurrentSelection()->data.wmo->pos.z, 0 );
		gWorld->GetVertex( gWorld->GetCurrentSelection()->data.wmo->pos.x, gWorld->GetCurrentSelection()->data.wmo->pos.z, &t );
		gWorld->GetCurrentSelection()->data.wmo->pos = t;
	}
	else if( gWorld->IsSelection( eEntry_Model ) )
	{
		Vec3D t = Vec3D( gWorld->GetCurrentSelection()->data.model->pos.x, gWorld->GetCurrentSelection()->data.model->pos.z, 0 );
		gWorld->GetVertex( gWorld->GetCurrentSelection()->data.model->pos.x, gWorld->GetCurrentSelection()->data.model->pos.z, &t );
		gWorld->GetCurrentSelection()->data.model->pos = t;				
	}
}

void CopySelectedObject( frame *button, int id )
{
	//! \todo	copy selected object path to clipboard
	if( gWorld->HasSelection() )
	{
		Environment::getInstance()->set_clipboard( gWorld->GetCurrentSelection() );
	}
}

void PasteSelectedObject( frame *button, int id )
{
	// MODELCOPY
	// paste object
	// if selection then insert on selection cords
	// else on current chunk cords
	// TODO
	if( gWorld->HasSelection() )
	{	
		nameEntry lClipboard = Environment::getInstance()->get_clipboard();
		switch( gWorld->GetCurrentSelection()->type )
		 {
			case eEntry_Model:
				gWorld->addModel( lClipboard, gWorld->GetCurrentSelection()->data.model->pos );
				break;
			case eEntry_WMO:
				gWorld->addModel( lClipboard, gWorld->GetCurrentSelection()->data.wmo->pos);
				break;
			case eEntry_MapChunk:
				gWorld->addModel( lClipboard, gWorld->GetCurrentSelection()->data.mapchunk->GetSelectionPosition() );
				break;
			default: break;
		}	
	}
}

void DeleteSelectedObject( frame *button, int id )
{
	if( gWorld->IsSelection( eEntry_WMO ) )
		gWorld->deleteWMOInstance( gWorld->GetCurrentSelection()->data.wmo->mUniqueID );
	else if( gWorld->IsSelection( eEntry_Model ) )
		gWorld->deleteModelInstance( gWorld->GetCurrentSelection()->data.model->d1 );
}

void InsertObject( frame *button, int id )
{
	// ID switch the import way


	// Test if there is an selection
	if( !gWorld->HasSelection() )
		return;
	// the list of the models to import
	std::vector<std::string> m2s_to_add;
	std::vector<std::string> wmos_to_add;

	// the import file
	std::string importFile;

	// MODELINSERT FROM TEXTFILE
	// is a source file set in config file?
	
		
		// insert from modelviewer if file set and hit the right menu pount.
		if(id==0)
			if( FileExists( "noggIt.conf" ) )
			{
				ConfigFile config( "noggIt.conf" );
				config.readInto( importFile, "ImportFile" );
			}
		// insert from import.txt if file exists and hit the right menu pount.
		if (id==1)
		{
			importFile="Import.txt"; //	use import.txt in noggit folder!
		}
	
		if(importFile=="")
			return;

	size_t foundString;
	std::string line;
	std::string findThis;
	std::ifstream fileReader(importFile.c_str());
	if (fileReader.is_open())
	{
		while (! fileReader.eof() )
		{
			getline (fileReader,line);
			if(line.find(".m2")!= std::string::npos || line.find(".M2")!= std::string::npos )
			{
				// M2 inside line
				// is it the modelviewer log then cut the log messages out
				findThis = 	"Loading model: ";
				foundString = line.find(findThis);
				if(foundString!= std::string::npos)
				{
					// cut path
					line = line.substr( foundString+findThis.size() );
				}
				m2s_to_add.push_back( line );
			}
			else if(line.find(".wmo")!= std::string::npos || line.find(".WMO")!= std::string::npos )
			{
				// WMO inside line
				findThis = "Loading WMOModel ";
				foundString = line.find(findThis);
				// is it the modelviewer log then cut the log messages out
				if(foundString != std::string::npos)
				{
					// cut path
					line = line.substr( foundString+findThis.size() );
				}
				wmos_to_add.push_back(line);
			}
		}
		fileReader.close();
	}
	else 
	{
		// file not exist, no rights ore other error
		LogError << importFile << std::endl;
	}
	
	Vec3D selectionPosition;
	switch( gWorld->GetCurrentSelection()->type )
	{
		case eEntry_Model:
			selectionPosition = gWorld->GetCurrentSelection()->data.model->pos;
			break;
		case eEntry_WMO:
			selectionPosition = gWorld->GetCurrentSelection()->data.wmo->pos;
			break;
		case eEntry_MapChunk:
			selectionPosition = gWorld->GetCurrentSelection()->data.mapchunk->GetSelectionPosition();
			break;
	}

	for( std::vector<std::string>::iterator it = wmos_to_add.begin(); it != wmos_to_add.end(); ++it )
	{
		
		if( !MPQFile::exists(*it) )
		{
			LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
			continue;
		}
		
		gWorld->addWMO( reinterpret_cast<WMO*>(WMOManager::items[WMOManager::add(*it)]), selectionPosition );
	}

	for( std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it )
	{

		if( !MPQFile::exists(*it) )
		{
			LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
			continue;
		}

		gWorld->addM2( reinterpret_cast<Model*>(ModelManager::items[ModelManager::add(*it)]), selectionPosition );
	}
	
	//! \todo Memoryleak: These models will never get deleted.
}

void view_texture_palette( frame *button, int id )
{
	TexturePalette->hidden = !TexturePalette->hidden;
}

void Toolbar_SelectIcon( frame * pButton, int pId )
{
	const char * Names[] = { "Raise / Lower", "Flatten / Blur", "3D Paint", "Holes", "Not used", "Not used", "Not used", "Not used", "Not used", "Not used" };
	change_settings_window( mainGui->guiToolbar->selectedIcon, pId > 3 ? 0 : pId );
	mainGui->guiToolbar->IconSelect( pId - 1 );
	mainGui->guiToolbar->text->setText( Names[pId-1] );
	terrainMode = (pId - 1) & 3;
	if (terrainMode == 3)
		Environment::getInstance()->view_holelines = true;
	else
		Environment::getInstance()->view_holelines = false;
}

void exit_tilemode( frame *button, int id )
{
	gPop = true;
}



//dirty hack
int round(float d)
{
	return d<0 ? d-.5f : d+.5f;
}
MapView::MapView(float ah0, float av0): ah(ah0), av(av0), mTimespeed( 0.0f )
{
	LastClicked=0;

	moving = strafing = updown = 0;

	mousedir = -1.0f;

	movespd = SPEED;

	lastBrushUpdate = 0;
	textureBrush.init();

	look = false;
	hud = true;
	set_areaid = false;
	mViewMode = eViewMode_3D;

	tileFrames = new frame( 0.0f, 0.0f, video.xres, video.yres );

	// create main gui opject that holds all other gui elements for access
	mainGui = new Gui();

	//register toolbar event functions
	for( int i = 0; i < 10; ++i )
		if( mainGui->guiToolbar->mToolbarIcons[i] )
			mainGui->guiToolbar->mToolbarIcons[i]->setClickFunc( Toolbar_SelectIcon, i+1 );

	mainGui->guiToolbar->current_texture->setClickFunc( view_texture_palette, 0 );
	tileFrames->addChild(mainGui->guiToolbar);

	// register statusbar
	tileFrames->addChild(mainGui->guiStatusbar);

	// register DetailInfo Window	
	tileFrames->addChild(mainGui->guidetailInfos);

	// register DetailInfo Window	
	tileFrames->addChild(mainGui->guiappInfo);

	// Scroll test window
	//tileFrames->addChild(mainGui->scrollPan);

	tool_settings_x = video.xres-186;
	tool_settings_y = 38;
	
	// Raisen/Lower
	setting_ground=new window(tool_settings_x,tool_settings_y,180.0f,160.0f);
	setting_ground->movable=true;
	tileFrames->addChild(setting_ground);

	setting_ground->addChild( new textUI( 78.5f, 2.0f, "Raise / Lower", &arial14, eJustifyCenter ) );
	
	gGroundToggleGroup = new ToggleGroup( &groundBrushType );
	setting_ground->addChild( new checkboxUI( 6.0f, 15.0f, "Flat", gGroundToggleGroup, 0 ) );
	setting_ground->addChild( new checkboxUI( 85.0f, 15.0f, "Linear", gGroundToggleGroup, 1 ) );
	setting_ground->addChild( new checkboxUI( 6.0f, 40.0f, "Smooth", gGroundToggleGroup, 2 ) );
	setting_ground->addChild( new checkboxUI( 85.0f, 40.0f, "Polynomial", gGroundToggleGroup, 3 ) );
	setting_ground->addChild( new checkboxUI( 6.0f, 65.0f, "Trigonom", gGroundToggleGroup, 4 ) );
	setting_ground->addChild( new checkboxUI( 85.0f, 65.0f, "Quadratic", gGroundToggleGroup, 5 ) );
	gGroundToggleGroup->Activate( 2 );

	ground_brush_radius=new slider(6.0f,120.0f,167.0f,1000.0f,0.00001f);
	ground_brush_radius->setFunc(setGroundBrushRadius);
	ground_brush_radius->setValue(groundBrushRadius/1000);
	ground_brush_radius->setText("Brush radius: %.2f");
	setting_ground->addChild(ground_brush_radius);

	ground_brush_speed=new slider(6.0f,145.0f,167.0f,10.0f,0.00001f);
	ground_brush_speed->setFunc(setGroundBrushSpeed);
	ground_brush_speed->setValue(groundBrushSpeed/10);
	ground_brush_speed->setText("Brush Speed: %.2f");
	setting_ground->addChild(ground_brush_speed);

	// flatten/blur
	setting_blur=new window(tool_settings_x,tool_settings_y,180.0f,100.0f);
	setting_blur->movable=true;
	setting_blur->hidden=true;
	tileFrames->addChild(setting_blur);

	setting_blur->addChild( new textUI( 78.5f, 2.0f, "Flatten / Blur", &arial14, eJustifyCenter ) );

	gBlurToggleGroup = new ToggleGroup( &blurBrushType );
	setting_blur->addChild( new checkboxUI( 6.0f, 15.0f, "Flat", gBlurToggleGroup, 0 ) );
	setting_blur->addChild( new checkboxUI( 80.0f, 15.0f, "Linear", gBlurToggleGroup, 1 ) );
	setting_blur->addChild( new checkboxUI( 6.0f, 40.0f, "Smooth", gBlurToggleGroup, 2 ) );
	gBlurToggleGroup->Activate( 2 );

	blur_brush=new slider(6.0f,85.0f,167.0f,1000.0f,0.00001f);
	blur_brush->setFunc(setBlurBrushRadius);
	blur_brush->setValue(blurBrushRadius/1000);
	blur_brush->setText("Brush radius: %.2f");
	setting_blur->addChild(blur_brush);

	//3D Paint settings window
	settings_paint=new window(tool_settings_x,tool_settings_y,180.0f,100.0f);
	settings_paint->hidden=true;
	settings_paint->movable=true;

	tileFrames->addChild(settings_paint);

	settings_paint->addChild( new textUI( 78.5f, 2.0f, "3D Paint", &arial14, eJustifyCenter ) );
	
	gradient *G1;
	G1=new gradient;	
	G1->width=20.0f;
	G1->x=settings_paint->width-4-G1->width;
	G1->y=4.0f;
	G1->height=92.0f;
	G1->setMaxColor(1.0f,1.0f,1.0f,1.0f);
	G1->setMinColor(0.0f,0.0f,0.0f,1.0f);
	G1->horiz=false;
	G1->setClickColor(1.0f,0.0f,0.0f,1.0f);
	G1->setClickFunc(setTextureBrushLevel);
	G1->setValue(0.0f);
	
	settings_paint->addChild(G1);

	slider	*S1;
	S1=new slider(6.0f,33.0f,145.0f,1.0f,0.0f);
	S1->setFunc(setTextureBrushHardness);
	S1->setValue(textureBrush.getHardness());
	S1->setText("Hardness: %.2f");
	settings_paint->addChild(S1);

	paint_brush=new slider(6.0f,59.0f,145.0f,100.0f,0.00001);
	paint_brush->setFunc(setTextureBrushRadius);
	paint_brush->setValue(textureBrush.getRadius() / 100 );
	paint_brush->setText("Radius: %.1f");
	settings_paint->addChild(paint_brush);

	S1=new slider(6.0f,85.0f,145.0f,0.99f,0.01f);
	S1->setFunc(setTextureBrushPressure);
	S1->setValue(brushPressure);
	S1->setText("Pressure: %.2f");
	settings_paint->addChild(S1);

	tileFrames->addChild(TexturePalette = TexturingUI::createTexturePalette(4,8,mainGui));
	TexturePalette->hidden=true;
	tileFrames->addChild(SelectedTexture = TexturingUI::createSelectedTexture());
	SelectedTexture->hidden=true;
	tileFrames->addChild(TexturingUI::createTilesetLoader());
	tileFrames->addChild(TexturingUI::createTextureFilter());
	tileFrames->addChild(MapChunkWindow = TexturingUI::createMapChunkWindow());
	MapChunkWindow->hidden=true;

	// create the menu
	menuBar * mbar = new menuBar();

	mbar->AddMenu( "File" );
	mbar->AddMenu( "Edit" );
	mbar->AddMenu( "View" );
	mbar->AddMenu( "Assist" );
	mbar->AddMenu( "Help" );

	mbar->GetMenu( "File" )->AddMenuItemButton( "CTRL + S Save current tile", SaveOrReload, 0 );
	mbar->GetMenu( "File" )->AddMenuItemButton( "SHIFT + J Reload current tile", SaveOrReload, 1 );
	mbar->GetMenu( "File" )->AddMenuItemButton( "ESC Exit", exit_tilemode, 0 );

	mbar->GetMenu( "Edit" )->AddMenuItemSeperator( "selected object" );
	mbar->GetMenu( "Edit" )->AddMenuItemButton( "STRG + C copy", CopySelectedObject, 0	);
	mbar->GetMenu( "Edit" )->AddMenuItemButton( "STRG + V past", PasteSelectedObject, 0	);
	mbar->GetMenu( "Edit" )->AddMenuItemButton( "DEL delete", DeleteSelectedObject, 0	);
	mbar->GetMenu( "Edit" )->AddMenuItemButton( "CTRL + R reset rotation", ResetSelectedObjectRotation, 0 );
	mbar->GetMenu( "Edit" )->AddMenuItemButton( "PAGE DOWN set to ground", SnapSelectedObjectToGround, 0 );
	mbar->GetMenu( "Edit" )->AddMenuItemSeperator( "m2 copy options" );
	mbar->GetMenu( "Edit" )->AddMenuItemToggle( "copy random rotation", &Settings::getInstance()->copy_rot, false	);
	mbar->GetMenu( "Edit" )->AddMenuItemToggle( "copy random tile", &Settings::getInstance()->copy_tile, false	);
	mbar->GetMenu( "Edit" )->AddMenuItemToggle( "copy random size", &Settings::getInstance()->copy_size, false	);
	
	mbar->GetMenu( "Edit" )->AddMenuItemSeperator( "Options" );
	mbar->GetMenu( "Edit" )->AddMenuItemToggle( "Auto select mode", &Settings::getInstance()->AutoSelectingMode, false );

	mbar->GetMenu( "Assist" )->AddMenuItemSeperator( "Import model from" );
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "ModelViewer", InsertObject, 0	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Text File", InsertObject, 1	);
	//mbar->GetMenu( "Assist" )->AddMenuItemSeperator( "Set" );
	//mbar->GetMenu( "Assist" )->AddMenuItemToggle( "Area ID", &set_areaid, true	);

	mbar->GetMenu( "View" )->AddMenuItemSeperator( "Windows" );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "Toolbar", &mainGui->guiToolbar->hidden, true );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "Current texture", &SelectedTexture->hidden, true );
	// Hide till its reimplemented.
	//mbar->GetMenu( "View" )->AddMenuItemToggle( "Map chunk settings", &MapChunkWindow->hidden, true );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "Texture palette", &TexturePalette->hidden, true );
	mbar->GetMenu( "View" )->AddMenuItemSeperator( "Toggle" );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F1 M2s", &gWorld->drawmodels );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F2 WMO doodadsets", &gWorld->drawdoodads );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F3 Terrain", &gWorld->drawterrain );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F4 Water", &gWorld->drawwater );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F6 WMOs", &gWorld->drawwmo );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F7 Lines", &gWorld->drawlines );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F8 Detail infos", &mainGui->guidetailInfos->hidden, true );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F9 Map contour infos", &DrawMapContour );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "F Fog", &gWorld->drawfog );
	mbar->GetMenu( "View" )->AddMenuItemToggle( "Holelines always on", &Settings::getInstance()->holelinesOn, false );

	mbar->GetMenu( "Help" )->AddMenuItemButton( "Key Bindings", openHelp, 0 );
	mbar->GetMenu( "Help" )->AddMenuItemToggle( "Infos", &mainGui->guiappInfo->hidden, true );

	tileFrames->addChild( mbar );
}

MapView::~MapView()
{
	if( mainGui )
	{
		delete mainGui;
		mainGui = NULL;
	}
	if( gWorld )
	{
		delete gWorld;
		gWorld = NULL;
	}
	if( tileFrames )
	{
		delete tileFrames;
		tileFrames = NULL;
	}
}

void MapView::tick( float t, float dt )
{
	Vec3D ObjPos;
	
	if( dt > 1.0f )
		dt = 1.0f;
		
	if( SDL_GetAppState() & SDL_APPINPUTFOCUS )
	{
		Vec3D dir( 1.0f, 0.0f, 0.0f );
		Vec3D dirUp( 1.0f, 0.0f, 0.0f );
		Vec3D dirRight( 0.0f, 0.0f, 1.0f );
		rotate( 0.0f, 0.0f, &dir.x,&dir.y, av * PI / 180.0f );
		rotate( 0.0f, 0.0f, &dir.x,&dir.z, ah * PI / 180.0f );

		if( Environment::getInstance()->ShiftDown )
		{
			dirUp.x = 0.0f;
			dirUp.y = 1.0f;
			dirRight *= 0.0f; //! \todo	WAT?
		}
		else if( Environment::getInstance()->CtrlDown )
		{
			dirUp.x = 0.0f;
			dirUp.y = 1.0f;
			rotate( 0.0f, 0.0f, &dirUp.x, &dirUp.y, av * PI / 180.0f );
			rotate( 0.0f, 0.0f, &dirRight.x, &dirRight.y, av * PI / 180.0f );
			rotate( 0.0f, 0.0f, &dirUp.x, &dirUp.z, ah * PI / 180.0f );
			rotate( 0.0f, 0.0f, &dirRight.x, &dirRight.z, ah * PI / 180.0f );
			
		}
		else
		{
			rotate( 0.0f, 0.0f, &dirUp.x, &dirUp.z, ah * PI / 180.0f );
			rotate( 0.0f, 0.0f, &dirRight.x, &dirRight.z, ah * PI / 180.0f );
		}

		nameEntry * Selection = gWorld->GetCurrentSelection();

		if( Selection )
		{
			if( keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0) 
			{
				// Move scale and rotat with numpad keys
				if( Selection->type == eEntry_WMO )
				{	
					Selection->data.wmo->pos.x += keyx * moveratio;
					Selection->data.wmo->pos.y += keyy * moveratio;
					Selection->data.wmo->pos.z += keyz * moveratio;
					Selection->data.wmo->dir.y += keyr * moveratio * 2;
				}

				if( Selection->type == eEntry_Model )
				{
					Selection->data.model->pos.x += keyx * moveratio;
					Selection->data.model->pos.y += keyy * moveratio;
					Selection->data.model->pos.z += keyz * moveratio;
					Selection->data.model->dir.y += keyr * moveratio * 2;
					Selection->data.model->sc += keys * moveratio / 50;
				}
			}

			if( gWorld->IsSelection( eEntry_Model ) )
			{
				//! \todo	Tell me what this is.
				ObjPos = Selection->data.model->pos - gWorld->camera;
				rotate( 0.0f, 0.0f, &ObjPos.x, &ObjPos.y, av * PI / 180.0f );
				rotate( 0.0f, 0.0f, &ObjPos.x, &ObjPos.z, ah * PI / 180.0f );
				ObjPos.x = abs( ObjPos.x );
			}
			
			// moving and scaling objects
			//! \todo	Alternatively automatically align it to the terrain. Also try to move it where the mouse points.
			if( MoveObj )
				if( Selection->type == eEntry_WMO )
				{
					 ObjPos.x = 80.0f;
					 Selection->data.wmo->pos+=mv * dirUp * ObjPos.x;
					 Selection->data.wmo->pos-=mh * dirRight * ObjPos.x;
					 Selection->data.wmo->extents[0] = Selection->data.wmo->pos - Vec3D(1,1,1);
					 Selection->data.wmo->extents[1] = Selection->data.wmo->pos + Vec3D(1,1,1);
				}
				else if( Selection->type == eEntry_Model )
					if( Environment::getInstance()->AltDown )
					{
						float ScaleAmount;
						ScaleAmount = pow( 2.0f, mv * 4.0f );
						Selection->data.model->sc *= ScaleAmount;
						if(Selection->data.model->sc > 63.9f )
							Selection->data.model->sc = 63.9f;
						else if (Selection->data.model->sc < 0.00098f )
							Selection->data.model->sc = 0.00098f;
					}
					else
					{
						ObjPos.x = 80.0f;
						Selection->data.model->pos += mv * dirUp * ObjPos.x;
						Selection->data.model->pos -= mh * dirRight * ObjPos.x;
					}
		

			// rotating objects
			if( look )
			{
				float * lTarget = NULL; 
				bool lModify = false;
				
				if( Selection->type == eEntry_Model )
				{
					lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
					if( Environment::getInstance()->ShiftDown )
						lTarget = &Selection->data.model->dir.y;
					else if( Environment::getInstance()->CtrlDown )
						lTarget = &Selection->data.model->dir.x;
					else if(Environment::getInstance()->AltDown )
						lTarget = &Selection->data.model->dir.z;
				}
				else if( Selection->type == eEntry_WMO )
				{
					lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
					if( Environment::getInstance()->ShiftDown )
						lTarget = &Selection->data.wmo->dir.y;
					else if( Environment::getInstance()->CtrlDown )
						lTarget = &Selection->data.wmo->dir.x;
					else if( Environment::getInstance()->AltDown )
						lTarget = &Selection->data.wmo->dir.z;
				}
				
				if( lModify )
				{
					*lTarget = *lTarget + rh + rv;

					if( *lTarget > 360.0f )
						*lTarget = *lTarget - 360.0f;
					else if( *lTarget < -360.0f )
						*lTarget = *lTarget + 360.0f;
				}
			}
		
			mh = 0;
			mv = 0;
			rh = 0;
			rv = 0;

			if( leftMouse && Selection->type==eEntry_MapChunk )
			{
				float xPos, yPos, zPos;
				Selection->data.mapchunk->getSelectionCoord( &xPos, &zPos );
				yPos = Selection->data.mapchunk->getSelectionHeight();
				
				switch( terrainMode )
				{
				case 0:
					if( Environment::getInstance()->ShiftDown )
						gWorld->changeTerrain( xPos, zPos, 7.5f * dt * groundBrushSpeed, groundBrushRadius, groundBrushType );
					else if( Environment::getInstance()->CtrlDown )
						gWorld->changeTerrain( xPos, zPos, -7.5f * dt * groundBrushSpeed, groundBrushRadius, groundBrushType );
						
					break;
					
				case 1:
					if( Environment::getInstance()->ShiftDown )
						gWorld->flattenTerrain( xPos, zPos, yPos, pow( 0.2f, dt ), blurBrushRadius, blurBrushType );
					else if( Environment::getInstance()->CtrlDown )
					{
						using std::min;
						gWorld->blurTerrain( xPos, zPos, pow( 0.2f, dt ), min( blurBrushRadius, 30.0f ), blurBrushType );
					}
					
					break;
					
				case 2:
					if( Environment::getInstance()->ShiftDown ){ // 3D Paint
						if( Environment::getInstance()->CtrlDown ) // clear chunk texture
							gWorld->eraseTextures(xPos, zPos);
						else if( TexturingUI::getSelectedTexture() ){
							if( textureBrush.needUpdate() )
								textureBrush.GenerateTexture();
							
							if( !gWorld->paintTexture( xPos, zPos, &textureBrush, brushLevel, 1.0f - pow( 1.0f - brushPressure, dt * 10.0f ), TextureManager::add( TexturingUI::getSelectedTexture()->name ) ) )
								LogError << "paintTexture failed ._. " << std::endl;
						}
						else{
							Log <<"Texture Pointer: "<< TexturingUI::getSelectedTexture(); //oO
						}
					}
						
					break;
					
				case 3:
					if( Environment::getInstance()->ShiftDown	)
						gWorld->removeHole( xPos, zPos );
					else if( Environment::getInstance()->CtrlDown )
						gWorld->addHole( xPos, zPos );
						
					break;
				}
			}
		}
		
		if( mViewMode != eViewMode_2D )
		{
			if( moving ) 
				gWorld->camera += dir * dt * movespd * moving;
			if( strafing ) 
			{
				Vec3D right = dir % Vec3D( 0.0f, 1.0f ,0.0f );
				right.normalize();
				gWorld->camera += right * dt * movespd * strafing;
			}
			if( updown ) 
				gWorld->camera += Vec3D( 0.0f, dt * movespd * updown, 0.0f );
			
			gWorld->lookat = gWorld->camera + dir;
		}
		else
		{
			if( moving ) 
				gWorld->camera.z -= dt * movespd * moving / ( gWorld->zoom * 1.5f );
			if( strafing ) 
				gWorld->camera.x += dt * movespd * strafing / ( gWorld->zoom * 1.5f );
			if( updown ) 
				gWorld->zoom *= pow( 2.0f, dt * updown * 4.0f );
			
			using std::min;
			using std::max;
			gWorld->zoom = min( max( gWorld->zoom, 0.1f ), 2.0f );

			if( leftMouse && !LastClicked && TexturingUI::getSelectedTexture() )
			{
				// Da bin ich überfragt, keine Ahnung, was der Code macht :) Dann bin ich ja nicht alleine :)
				// Aber grundsätzlich wird hier die Mouseposition errechnet. Auf mir unbekannte Art und weise.
				// Mal was schauen, momen
				float mX, mY;
				mX = CHUNKSIZE * 4.0f * video.ratio * ( float( MouseX ) / float( video.xres ) - 0.5f ) / gWorld->zoom+gWorld->camera.x;
				mY = CHUNKSIZE * 4.0f * ( float( MouseY ) / float( video.yres ) - 0.5f) / gWorld->zoom+gWorld->camera.z;

				if( Environment::getInstance()->CtrlDown )
					gWorld->eraseTextures( mX, mY );
				else
				{		
					if( textureBrush.needUpdate() )
						textureBrush.GenerateTexture();
					
					gWorld->paintTexture( mX, mY, &textureBrush, brushLevel, 1.0f - pow( 1.0f - brushPressure, dt * 10.0f ), TextureManager::add( TexturingUI::getSelectedTexture() ->name ) );
				}
			}

		}
	}
	else
	{
		leftMouse = false;
		rightMouse = false;
		look = false;
		MoveObj = false;
		moving = 0;
		strafing = 0;
		updown = 0;
	}

	if( ( t - lastBrushUpdate ) > 0.1f && textureBrush.needUpdate() )
		textureBrush.GenerateTexture();

	gWorld->time += this->mTimespeed * dt;
	gWorld->animtime += dt * 1000.0f;
	globalTime = (int)gWorld->animtime;
	
	gWorld->tick(dt);
}

void MapView::doSelection( int selTyp )
{
	gWorld->drawSelection( MouseX, MouseY, TestSelection );
	gWorld->getSelection( eSelectionMode_General );
	
	if( gWorld->GetCurrentSelection() )
	{
		if( gWorld->IsSelection( eEntry_MapChunk ) )
		{
			gWorld->drawSelectionChunk( MouseX, MouseY );
			gWorld->getSelection( eSelectionMode_Triangle );

			Environment::getInstance()->AutoSelecting = true;
		}
		else if( selTyp == 1 )
		{
			Environment::getInstance()->AutoSelecting = false;
		}
		else if( Environment::getInstance()->AutoSelecting )
		{
			gWorld->ResetSelection();
		}
	}
	else
		Environment::getInstance()->AutoSelecting = true;
}

void MapView::displayViewMode_Help( float t, float dt )
{
	//! \todo	Make this a window instead of a view. Why should you do it as a view? ._.
	video.clearScreen();
	video.set2D();
	glEnable(GL_TEXTURE_2D);
		glColor4f(0.7f,0.7f,0.7f,0.7f);
		glBindTexture(GL_TEXTURE_2D,TextureManager::add("DUNGEONS\\TEXTURES\\BRIAN\\CLASSICALELFRUINS\\AZR_WINDOW01.BLP") );
		glBegin(GL_QUADS);		
			glTexCoord2f(0.0f,0.0f);
			glVertex2i(0.0f,0.0f);
			glTexCoord2f(1.0f,0.0f);
			glVertex2i(video.xres,0.0f);
			glTexCoord2f(1.0f,1.0f);
			glVertex2i(video.xres,video.yres);
			glTexCoord2f(0.0f,1.0f);
			glVertex2i(0.0f,video.yres);
		glEnd();
	glDisable(GL_TEXTURE_2D);
	

	freetype::shprint( morpheus40, (video.xres / 2) - freetype::width( morpheus40, "NoggIt shortcuts" )/2.0f, 44.0f, "NoggIt shortcuts" );
	freetype::shprint( arial16, (video.xres / 2) - freetype::width( arial16, "Right click close the help." )/2.0f, 87.0f, "Right click close the help." );
	
	freetype::shprint( arial24, 74.0f, 120.0f, "Basic controles" );
	freetype::shprint( arial16, 74.0f, 160.0f, 
						"Left mouse button moves the camera\n"
			"I - invert mouse up and down\n"
			"Q,E - move up,down\n"
			"A,D,W,S - move left,right,forward,backward\n"
			"R - turn camera 180 degres\n"
			"M - show minimap\n"
			"U - 2d texture editor\n"
			"C - chunk settings\n"
			"H - help\n"
			"Mouse left click - select chunk or object\n"
			"Shift + F4 - change to auto select mode\n"
			"Esc - exit to main menu"
	);

	freetype::shprint( arial24, 74.0f, 380.0f, "Toggles" );
	freetype::shprint( arial16, 74.0f, 420.0f, 
			"F1 - toggle M2s\n"
			"F2 - toggle WMO doodads set\n" 
			"F3 - toggle ground\n"
			"F4 - toggle GUI\n"
			"F6 - toggle WMOs\n"
			"F7 - toggle chunk (red) and ADT (green) lines\n"
			"F8 - toggle detailed infotext\n"
			"F9 - toggle map contour\n"
			"F	- toggle fog\n"
			"TAB - toggle UI view"
	);		
		
	freetype::shprint( arial24, 74.0f, 620.0f, "Adjust" );
	freetype::shprint( arial16, 74.0f, 660.0f, 		
			"+,- - adjust fog distance\n"
			"O,P - slower/faster movement\n"
			"B,N - slower/faster time\n"
	);		
	

	freetype::shprint( arial24, video.xres - 400.0f, 130.0f, "Edit ground" );
	freetype::shprint( arial16, video.xres - 400.0f, 170.0f, 
			"Shift + F1 - toggle ground edit mode\n"
			"T - change terrain mode\n"
			"Y - changes brush type\n"
			"ALT + left mouse + mouse move - change brush size\n"
			"Terrain mode - raise/lower\n"
			"	Left mouse click + Shift - raise terrain\n"
			"	Left mouse click + Alt	 - lower terrain\n"
			"Terrain mode - flatten/blur\n"
			"	Left mouse click + Shift - flatten terrain\n"
			"	Left mouse	click + Alt	 - blur terrain\n"			
	);

	freetype::shprint( arial24, video.xres - 400.0f, 360.0f, "Edit objects" );
	freetype::shprint( arial16, video.xres - 400.0f, 400.0f,
			"An object must be selected by mouse left click\n"
			"Hold middle mouse - move object\n"
			"Hold middle mouse + Alt - scale M2\n"
			"Hold left mouse + Shift, Ctrl or Alt - rotate object\n"
			"0-9 - change doodads set of selected WMO\n"
			"Ctrl+R - Reset Rotation\n"
			"PageDown - Set Object to Groundlevel\n"
	);	
	freetype::shprint( arial24, video.xres - 400.0f, 525.0f, "Edit texture" );
	freetype::shprint( arial16, video.xres - 400.0f, 550.0f,
			"Hold CTRL + Shift - draw texture in 3D mode\n"
			"Z - chunk texture replace"
	);

	freetype::shprint( arial24, video.xres - 400.0f, 590.0f, "Files" );
	freetype::shprint( arial16, video.xres - 400.0f, 630.0f,
			"F5	- save bookmark\n"
			"F10 - reload BLP\n"
			"F11 - reload M2s\n"
			"F12 - reload wmo\n"
			"Shift + J - reload ADT tile\n"
			"NUM 0 - SAVE current ADT tile\n"
	);
}

void MapView::displayViewMode_Minimap( float t, float dt )
{
		//! \todo	try to use a real map from WoW? either the large map or the minimap would be nice
	video.clearScreen();
	video.set2D();

	const int len = 768;
	const int basex = 200;
	const int basey = 0;

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, gWorld->minimap);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);
			glVertex2i(basex,basey);
			glTexCoord2f(1.0f,0.0f);
			glVertex2i(basex+len,basey);
			glTexCoord2f(1.0f,1.0f);
			glVertex2i(basex+len,basey+len);
			glTexCoord2f(0.0f,1.0f);
			glVertex2i(basex,basey+len);
		glEnd();
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		float fx, fz;
		fx = basex + gWorld->camera.x / TILESIZE * 12.0f;
		fz = basey + gWorld->camera.z / TILESIZE * 12.0f;
		glVertex2f(fx, fz);
		glColor4f(1.0f,1.0f,1.0f,0.0f);
		glVertex2f(fx + 10.0f*cosf(ah/180.0f*PI), fz + 10.0f*sinf(ah/180.0f*PI));
	glEnd();

	//! \todo	Something is wrong there.
	//gWorld->skies->drawSky(Vec3D(0.0f,0.0f,0.0f));
}

void MapView::displayViewMode_2D( float t, float dt )
{
	video.setTileMode();
	gWorld->drawTileMode(ah);
	
	float mX,mY,tRadius;
	mX=4.0f*video.ratio*((float)MouseX/(float)video.xres-0.5f);
	mY=4.0f*((float)MouseY/(float)video.yres-0.5f);

	mX=CHUNKSIZE*4.0f*video.ratio*((float)MouseX/(float)video.xres-0.5f)/gWorld->zoom+gWorld->camera.x;
	mY=CHUNKSIZE*4.0f*((float)MouseY/(float)video.yres-0.5f)/gWorld->zoom+gWorld->camera.z;

	mX=mX/CHUNKSIZE;
	mY=mY/CHUNKSIZE;

	// draw brush
	glPushMatrix();
		glScalef(gWorld->zoom,gWorld->zoom,1.0f);
		glTranslatef(-gWorld->camera.x/CHUNKSIZE,-gWorld->camera.z/CHUNKSIZE,0);

		glColor4f(1.0f,1.0f,1.0f,0.5f);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, textureBrush.getTexture());
		tRadius=textureBrush.getRadius()/CHUNKSIZE;//*gWorld->zoom;
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);
			glVertex3f(mX-tRadius,mY+tRadius,0);
			glTexCoord2f(1.0f,0.0f);
			glVertex3f(mX+tRadius,mY+tRadius,0);
			glTexCoord2f(1.0f,1.0f);
			glVertex3f(mX+tRadius,mY-tRadius,0);
			glTexCoord2f(0.0f,1.0f);
			glVertex3f(mX-tRadius,mY-tRadius,0);
		glEnd();
	glPopMatrix();

	video.set2D();
	
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	tileFrames->render();
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	if (hud) 
	{
		freetype::shprint( arial16, 410.0f, 4.0f, gAreaDB.getAreaName( gWorld->getAreaID() ).c_str() );
		std::stringstream fps; fps << gFPS << " fps";
		freetype::shprint( arial16, video.xres - 200.0f, 5, "%.2f fps", gFPS );
	}

	if (gWorld->loading) 
		freetype::shprint( arial16, video.xres / 2 - freetype::width( arial16, gWorld->noadt ? "No ADT at this Point" : "Loading..." ) / 2, 30.0f, ( gWorld->noadt ? "No ADT at this Point" : "Loading..." ) );
}

void MapView::displayViewMode_3D( float t, float dt )
{
	video.set3D();

	if( Environment::getInstance()->AutoSelecting && Settings::getInstance()->AutoSelectingMode )
		doSelection(0);
	
	video.set3D();

	//glActiveTexture(GL_TEXTURE1);
	//glDisable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);

	gWorld->draw();


	
	if( hud ) 
	{
		video.set2D();
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glColor4f(1,1,1,1);
		
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		tileFrames->render();
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		freetype::shprint( arial16, 510, 4, gAreaDB.getAreaName( gWorld->getAreaID() ).c_str() );
		std::stringstream fps; fps << gFPS << " fps";
		freetype::shprint( arial16, video.xres - 200, 5, "%.2f fps", gFPS );
				
		std::ostringstream s;
		s << "Server cords(x:" << -(gWorld->camera.x - ZEROPOINT) << " y:" << -(gWorld->camera.z - ZEROPOINT) << " z:" << gWorld->camera.y
				<< ") Tile " << round((gWorld->camera.x-(TILESIZE/2))/TILESIZE) << " " << round((gWorld->camera.z-(TILESIZE/2))/TILESIZE)
				<< " Client cords(x:" << gWorld->camera.x<<" y:" << gWorld->camera.z << " z:"<<gWorld->camera.y << ") ";
		mainGui->guiStatusbar->setLeftInfo( s.str() );
		
		int time = int( gWorld->time ) % 2880;

		std::stringstream timestrs; timestrs << "Time: " << (time/120) << ":" << (time%120);
		freetype::shprint( arial16, video.xres - 100.0f, 5.0f, "Time:	%02d:%02d", time / 120, (time % 120) / 2 );
		
		if( mainGui->guiappInfo->hidden == false )
		{	
			s.str("");
			s << "Project Path: " << Project::getInstance()->getPath() << std::endl;
			mainGui->guiappInfo->setText( s.str() );
		}

		//! \todo	Get this into a window. As Steff is already doing.
		if( !mainGui->guidetailInfos->hidden )
		{
			nameEntry * lSelection = gWorld->GetCurrentSelection();
			if( lSelection )
			{
				if( !MapChunkWindow->hidden )
					TexturingUI::setChunkWindow( lSelection->data.mapchunk );

				//! \todo	Only do this if lSelection == Selection? ..
				mainGui->guiStatusbar->setRightInfo( lSelection->returnName() );

				s.str("");

				switch( lSelection->type ) 
				{
				case eEntry_Model:
						s << "TYPE: " << lSelection->type << std::endl;
					s << "NAME: " << lSelection->returnName()<< std::endl;
					s << "FILENAME: " << lSelection->data.model->model->filename << std::endl;

					s << "UID-D1: " << lSelection->data.model->d1 << std::endl;
//					s << "modelID: " << lSelection->data.model->modelID << std::endl;
					s << "nameID: " << lSelection->data.model->nameID << std::endl << std::endl;
					s << "Pos X: " << lSelection->data.model->pos.x << std::endl;
					s << "Pos Y: " << lSelection->data.model->pos.y << std::endl;
					s << "Pos Z: " << lSelection->data.model->pos.z << std::endl;
					//s << "UniqueID: " << lSelection->Name << endl;
					//s << "UniqueID: " << lSelection->Name << endl;
					//freetype::shprint( arial16, 10, 83, "UniqueID: %d", lSelection->data.model->d1 );
					//s <<	"Pos: (" <<	lSelection->data.model->pos.x << "," << lSelection->data.model->pos.y << "," << lSelection->data.model->pos.z << ")" << endl;					
					//freetype::shprint( arial16, 10, 103, "Pos: (%.2f, %.2f, %.2f)", lSelection->data.model->pos.x, lSelection->data.model->pos.y, lSelection->data.model->pos.z );
					//s << "Rot: (" << lSelection->data.model->dir.x << "," << lSelection->data.model->dir.y << "" << lSelection->data.model->dir.z << ")" << endl;					
					//freetype::shprint( arial16, 10, 123, "Rot: (%.2f, %.2f, %.2f)", lSelection->data.model->dir.x, lSelection->data.model->dir.y, lSelection->data.model->dir.z );
					//s << "Scale: " <<	lSelection->data.model->sc << endl;					
					//freteype::shprint( arial16, 10, 143, "Scale: %.2f", lSelection->data.model->sc );
					//s << "Textures Used: " << lSelection->data.model->model->header.nTextures << endl;					
					//freetype::shprint( arial16, 10, 163, "Textures Used: %d", lSelection->data.model->model->header.nTextures );
				

				//	for( unsigned int j = 0; j < lSelection->data.model->model->header.nTextures; j++ )
				//	{
						//s << j << ", " << lSelection->data.model->model->textures[j] << " - " << TextureManager::items[lSelection->data.model->model->textures[j]]->name << endl;WHY DID THIS CRASH!!!
						//freetype::shprint( arial16, 15, 183 + 20 * j, "%d - %s", j, TextureManager::items[lSelection->data.model->model->textures[j]]->name );
				//	}
					mainGui->guidetailInfos->setText(s.str() );
				break;
				case eEntry_WMO:
					s << lSelection->data.wmo->wmo->filename << std::endl;
					//freetype::shprint( arial16, 5, 63, lSelection->data.wmo->wmo->filename );
					s << "UniqueID: " << lSelection->data.wmo->mUniqueID << std::endl;
					//freetype::shprint( arial16, 10, 83, "UniqueID: %d", lSelection->data.wmo->id );
					s <<	"Pos: (" <<	lSelection->data.wmo->pos.x << "," << lSelection->data.wmo->pos.y << "," << lSelection->data.wmo->pos.z << ")" << std::endl;					
					//freetype::shprint( arial16, 10, 103, "Pos: (%.2f, %.2f, %.2f)", lSelection->data.wmo->pos.x, lSelection->data.wmo->pos.y, lSelection->data.wmo->pos.z );
					s << "Rot: (" << lSelection->data.wmo->dir.x << "," << lSelection->data.wmo->dir.y << "" << lSelection->data.wmo->dir.z << ")" << std::endl;					
					//freetype::shprint( arial16, 10, 123, "Rot: (%.2f, %.2f, %.2f)", lSelection->data.wmo->dir.x, lSelection->data.wmo->dir.y, lSelection->data.wmo->dir.z );
					s << "Textures Used: " << lSelection->data.wmo->wmo->nTextures << std::endl;
					//freetype::shprint( arial16, 10, 143, "Textures Used: %d", lSelection->data.model->model->header.nTextures );

				/*	for( unsigned int j = 0; j < lSelection->data.wmo->wmo->nTextures; j++ )
					{
						if( j < 25 )
						{
							//s << j << " - " << lSelection->data.wmo->wmo->textures[j] << endl; WHY DID THIS CRASH!!!
							//freetype::shprint( arial16, 15, 163 + 20 * j, "%d - %s", j, lSelection->data.wmo->wmo->textures[j] );
						}
						else if( j < 50 )
						{
							//s << j << " - " << lSelection->data.wmo->wmo->textures[j] << endl; WHY DID THIS CRASH!!!
							//freetype::shprint( arial16, ( video.xres - 15 ) / 2, 163 + 20 * (j-25), "%d - %s", j, lSelection->data.wmo->wmo->textures[j] );
						}
					}*/
					s << "Doodads set: " << lSelection->data.wmo->doodadset << std::endl;
					//freetype::shprint( arial16, 10, 143, "Doodads set: %d", lSelection->data.wmo->doodadset );

					mainGui->guidetailInfos->setText( s.str() );

					break;
				case eEntry_MapChunk:
					/* crash noggit in the moment???
					int TextOffset = 0;
					s << "Mapchunk " << lSelection->data.mapchunk->px << ", " << lSelection->data.mapchunk->py << " (" << lSelection->data.mapchunk->py * 16 + lSelection->data.mapchunk->px << ") of tile (" << lSelection->data.mapchunk->mt->x << "_" << lSelection->data.mapchunk->mt->z << ")" << endl;
					//freetype::shprint( arial16, 5, 63, "Mapchunk %d, %d (%d) of tile (%d_%d)", lSelection->data.mapchunk->px, lSelection->data.mapchunk->py, lSelection->data.mapchunk->py * 16 + lSelection->data.mapchunk->px, lSelection->data.mapchunk->mt->x, lSelection->data.mapchunk->mt->z );
					s << "Flags: " << lSelection->data.mapchunk->Flags << endl;
					//freetype::shprint( arial16, 10, 83, "Flags %x", lSelection->data.mapchunk->Flags );

					if( lSelection->data.mapchunk->Flags & FLAG_SHADOW )
					{
						s << "Shadows Enabled" << endl;
						//freetype::shprint( arial16, 15, 103 + TextOffset, "Shadows Enabled" );
						TextOffset += 20;
					}
					if( lSelection->data.mapchunk->Flags & FLAG_IMPASS )
					{
						s << "Impassible Chunk" << endl;
						//freetype::shprint( arial16, 15, 103 + TextOffset, "Impassible Chunk" );
						TextOffset += 20;
					}
					if( lSelection->data.mapchunk->Flags & FLAG_LQ_RIVER )
					{
						s << "River Enabled" << endl;
						//freetype::shprint( arial16, 15, 103 + TextOffset, "River Enabled" );
						TextOffset += 20;
					}
					if( lSelection->data.mapchunk->Flags & FLAG_LQ_OCEAN )
					{
						s << "Ocean Enabled" << endl;
						//freetype::shprint( arial16, 15, 103 + TextOffset, "Ocean Enabled" );
						TextOffset += 20;
					}
					if( lSelection->data.mapchunk->Flags & FLAG_LQ_MAGMA )
					{
						s << "Lava Enabled" << endl;
						//freetype::shprint( arial16, 15, 103 + TextOffset, "Lava Enabled" );
						TextOffset += 20;
					}

					s << "Textures: " << lSelection->data.mapchunk->nTextures << endl;
					//freetype::shprint( arial16, 10, 103 + TextOffset, "Textures %d", lSelection->data.mapchunk->nTextures );
					TextOffset += 20;
					for( int q = 0; q < lSelection->data.mapchunk->nTextures; q++ )
					{
						//s << q << "- " << TextureManager::items[lSelection->data.mapchunk->textures[q]]->name << "	Flags - " << lSelection->data.mapchunk->texFlags[q] << " Effect ID - " << lSelection->data.mapchunk->effectID[q] << endl; WHY DID THIS CRASH!!!
						//freetype::shprint( arial16, 20, 103 + TextOffset, "%d - %s	Flags - %x Effect ID - %d", q, TextureManager::items[lSelection->data.mapchunk->textures[q]]->name, lSelection->data.mapchunk->texFlags[q], lSelection->data.mapchunk->effectID[q] );
						TextOffset += 20;
						if( lSelection->data.mapchunk->effectID[q] != 0 )
							for( int r = 0; r < 4; r++ )
							{
								const char *EffectModel = getGroundEffectDoodad( lSelection->data.mapchunk->effectID[q], r );
								if( EffectModel )
								{
										s << r << " - World\\NoDXT\\" << EffectModel << endl;	
										//freetype::shprint( arial16, 30, 103 + TextOffset, "%d - World\\NoDXT\\%s", r, EffectModel );
										TextOffset += 20;
								}
							}
						TextOffset+=5;
					}
					
					
					mainGui->guidetailInfos->setText( s.str() );*/
					break;
				}
			}
			else 
			{
				mainGui->guidetailInfos->setText( "" );
			}
		}
	}

	if ( gWorld->loading ) 
	{
		video.set2D();
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glColor4f(1,1,1,1);

		freetype::shprint( arial16, video.xres/2 - freetype::width( arial16, gWorld->noadt ? "No ADT at this Point" : "Loading..." ) / 2, 30, ( gWorld->noadt ? "No ADT at this Point" : "Loading..." ) );
	}
}



void MapView::display( float t, float dt )
{
	//! \todo	Get this out or do it somehow else. This is ugly and is a senseless if each draw.
	if( Saving )
	{		
		video.setTileMode();
		gWorld->saveMap();
		Saving=false;
	}

	if(set_areaid)
	{
		// reset Area ID
	
	}


	switch( mViewMode )
	{
	case eViewMode_Help:
		displayViewMode_Help( t, dt );
		break;
	case eViewMode_Minimap:
		displayViewMode_Minimap( t, dt );
		break;
	case eViewMode_2D:
		displayViewMode_2D( t, dt );
		break;
	case eViewMode_3D:
		displayViewMode_3D( t, dt );

		break;
	}
			//CheckForGLError( "MapView::display(), after displayViewMode_3D" );

}



void MapView::resizewindow()
{
	for( std::vector<frame*>::iterator child = tileFrames->children.begin(); child != tileFrames->children.end(); ++child )
		if( (*child)->mustresize )
			(*child)->resize();
}

void MapView::keypressed( SDL_KeyboardEvent *e )
{
	if( !( SDL_GetAppState() & SDL_APPINPUTFOCUS ) )
		return;									// finally stop getting keys from chatting ..

	//if( textbox->KeyBoardEvent( e ) ) return;

	if( e->type == SDL_KEYDOWN ) 
	{
		if((LastClicked)&&(LastClicked->processKey(e->keysym.sym,(e->keysym.mod&KMOD_SHIFT)!=0,(e->keysym.mod&KMOD_CTRL)!=0,(e->keysym.mod&KMOD_ALT)!=0)))
			return;

		// key DOWN
		
		if( e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT )
			Environment::getInstance()->ShiftDown = true;

		if( e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT )
			Environment::getInstance()->AltDown = true;

		if( e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL )
			Environment::getInstance()->CtrlDown = true;

		// quit or leave help window
		if( e->keysym.sym == SDLK_ESCAPE )
			if( mViewMode == eViewMode_Help )
				mViewMode = eViewMode_3D;
			else
				gPop = true;

		// movement
		if( e->keysym.sym == SDLK_w )
			moving = 1.0f;
		
		// saving or movement
		//! \todo	Write ctrl-s in the help. Also get these idiots up to date so they dont complain about it not saving again.
		if( e->keysym.sym == SDLK_s )
			if( !Environment::getInstance()->CtrlDown )
				moving = -1.0f;
			else
				gWorld->saveTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );
		
		if( e->keysym.sym == SDLK_a )
			strafing = -1.0f;
		
		if( e->keysym.sym == SDLK_d )
			strafing = 1.0f;
		
		if( e->keysym.sym == SDLK_e )
			updown = -1.0f;
		
		if( e->keysym.sym == SDLK_q )
			updown = 1.0f;
		
		// position correction with numpad
		if( e->keysym.sym == SDLK_KP8 ) 
			keyx = -1;

		if( e->keysym.sym == SDLK_KP2 ) 
			keyx = 1;

		if( e->keysym.sym == SDLK_KP6 ) 
			keyz = -1;

		if( e->keysym.sym == SDLK_KP4 ) 
			keyz = 1;

		if( e->keysym.sym == SDLK_KP1 ) 
			keyy = -1;

		if( e->keysym.sym == SDLK_KP3 ) 
			keyy = 1;

		if( e->keysym.sym == SDLK_KP7 ) 
			keyr = 1;

		if( e->keysym.sym == SDLK_KP9 ) 
			keyr = -1;

		if( e->keysym.sym == SDLK_KP5 ) 
		{	
			 if (moveratio == 0.01f) moveratio=0.1f;			 
			 else if (moveratio == 0.1f) moveratio=0.2f;
			 else if (moveratio == 0.2f) moveratio=0.3f;
			 else if (moveratio == 0.3f) moveratio=0.01f;
		}

		// This was Bekets function to replace chunk textures. Redo.
	/*	if ((e->keysym.sym == SDLK_z)&&(Selection!=0)&&(Selection->type==eEntry_MapChunk)) 
		{
			setChunk(Selection->data.mapchunk);
		}*/

		// \todo Deletion kills noggit.
		// delete object
		if( e->keysym.sym == SDLK_DELETE )
			DeleteSelectedObject( 0, 0 );

		// open chunk settings window or copy & paste
		if( e->keysym.sym == SDLK_c)
		{	
			if( mViewMode == eViewMode_2D )
			{
				if( gWorld->IsSelection( eEntry_MapChunk ) )
				{
					tileFrames->addChild( TexturingUI::createMapChunkWindow() );
					TexturingUI::setChunkWindow( gWorld->GetCurrentSelection()->data.mapchunk );
					MapChunkWindow->hidden = false;
				}
			}
			else
				if( Environment::getInstance()->CtrlDown )
					CopySelectedObject( 0, 0 );
		}
		if( e->keysym.sym == SDLK_v && Environment::getInstance()->CtrlDown )
			PasteSelectedObject( 0, 0 );

		if( e->keysym.sym == SDLK_x )
		{
				/*if(Environment::getInstance()->CtrlDown)
				{
					// copy selected object to clipboard and delete selected object.
					// TODO
					if( Selection )
					{
						Environment::getInstance()->set_clipboard(Selection);
						if( Selection->type == eEntry_Model )
						{
							gWorld->deleteModelInstance( Selection->data.model );
						}
						else if( Selection->type == eEntry_WMO )
						{
							gWorld->deleteWMOInstance( Selection->data.wmo );
						}
						Selection = 0;
					}
				}
				*/
		}

		// invert mouse
		if( e->keysym.sym == SDLK_i )
			mousedir *= -1.0f;
		
		// move speed or raw saving (?)
		if( e->keysym.sym == SDLK_p )
			if( Environment::getInstance()->CtrlDown && Environment::getInstance()->ShiftDown )
				Saving = true;
			else
				movespd *= 2.0f;
		
		if( e->keysym.sym == SDLK_o )
			movespd *= 0.5f;
		
		// turn around or reset object orientation
		if( e->keysym.sym == SDLK_r )
			if( Environment::getInstance()->CtrlDown )
				ResetSelectedObjectRotation( 0, 0 );
			else
				ah += 180.0f;
		
		// clip object to ground
		if( e->keysym.sym == SDLK_PAGEDOWN )
			SnapSelectedObjectToGround( 0, 0 );

		// speed of daytime.
		if( e->keysym.sym == SDLK_n )
			this->mTimespeed += 90.0f;
		
		if( e->keysym.sym == SDLK_b )
			this->mTimespeed -= 90.0f;
		
		// toggle editing mode
		if( e->keysym.sym == SDLK_t ) 
		{
			terrainMode++;
			terrainMode = terrainMode % 4;

			// Set the right icon in toolbar
			Toolbar_SelectIcon( 0, terrainMode + 1 );
		}
		if( e->keysym.sym == SDLK_r ) 
		{
			terrainMode--;
			terrainMode = ( terrainMode + 4 ) % 4;

			// Set the right icon in toolbar
			Toolbar_SelectIcon( 0, terrainMode + 1 );
		}

		// toggle lightning
		if( e->keysym.sym == SDLK_l ) 
			gWorld->lighting = !gWorld->lighting;

		// toggle interface
		if( e->keysym.sym == SDLK_TAB )
			hud = !hud;	

		// toggle "terrain texturing mode" / draw models
		if( e->keysym.sym == SDLK_F1 )
			if( Environment::getInstance()->ShiftDown )
			{
				if( alloff )
				{
					alloff_models = gWorld->drawmodels;
					alloff_doodads = gWorld->drawdoodads;
					alloff_contour = DrawMapContour;
					alloff_wmo = gWorld->drawwmo;
					alloff_fog = gWorld->drawfog;
					alloff_terrain = gWorld->drawterrain;

					gWorld->drawmodels = false;
					gWorld->drawdoodads = false;
					DrawMapContour = true;
					gWorld->drawwmo = false;
					gWorld->drawterrain = true;
					gWorld->drawfog = false;
				}
				else
				{
					gWorld->drawmodels = alloff_models;
					gWorld->drawdoodads = alloff_doodads;
					DrawMapContour = alloff_contour;
					gWorld->drawwmo = alloff_wmo;
					gWorld->drawterrain = alloff_terrain;
					gWorld->drawfog = alloff_fog;
				}
				alloff = !alloff;
			}
			else
				gWorld->drawmodels = !gWorld->drawmodels;
	

		// toggle drawing of doodads in WMOs.
		if( e->keysym.sym == SDLK_F2 ) 
			gWorld->drawdoodads = !gWorld->drawdoodads;

		// toggle terrain
		if( e->keysym.sym == SDLK_F3 ) 
			gWorld->drawterrain = !gWorld->drawterrain;

		// toggle better selection mode
		if( e->keysym.sym == SDLK_F4 && Environment::getInstance()->ShiftDown )
		{				
			Settings::getInstance()->AutoSelectingMode = !Settings::getInstance()->AutoSelectingMode;
		}			
		
		// toggle better selection mode
		if( e->keysym.sym == SDLK_F4 && !Environment::getInstance()->ShiftDown )
				gWorld->drawwater = !gWorld->drawwater;

		// toggle chunk limitation lines
		if( e->keysym.sym == SDLK_F7 ) 
			if( Environment::getInstance()->ShiftDown )
			{
				Environment::getInstance()->view_holelines = !Environment::getInstance()->view_holelines;
			}
			else
			gWorld->drawlines = !gWorld->drawlines;

		// toggle drawing of WMOs
		if( e->keysym.sym == SDLK_F6 ) 
			gWorld->drawwmo = !gWorld->drawwmo;

		// toggle showing a lot of information about selected item
		if( e->keysym.sym == SDLK_F8 ) 
		{
			mainGui->guidetailInfos->hidden = !mainGui->guidetailInfos->hidden;
		}

		// toggle height contours on terrain
		if( e->keysym.sym == SDLK_F9 ) 
			DrawMapContour = !DrawMapContour;

		// help
		if( e->keysym.sym == SDLK_h )
			if( mViewMode == eViewMode_Help )
				mViewMode = eViewMode_3D;
			else
				mViewMode = eViewMode_Help;

		// draw fog
		if( e->keysym.sym == SDLK_f ) 
			gWorld->drawfog = !gWorld->drawfog;

		// reload a map tile
		if( e->keysym.sym == SDLK_j && Environment::getInstance()->ShiftDown )
			gWorld->reloadTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );

#ifdef DEBUG
		// Check for layers we have too much. this should filter out those that are completely covered.
		//! \todo	Get this function to work.
		if( e->keysym.sym == SDLK_k ) 
		{
			if((Selection!=0)&&(Selection->type==eEntry_MapChunk))
			{
				gLog( "This chunk has %i textures.\n", Selection->data.mapchunk->nTextures ); 
				for( int layer = 1; layer < Selection->data.mapchunk->nTextures; layer++ )
				{
					gLog( "\tTesting layer %i:\n", layer );
					bool used = false;
					for( int j = 0; j < 63 && !used; j++ )
						for( int i = 0; i < 63 && !used; ++i )
							if( Selection->data.mapchunk->amap[layer][i + j * 64] != 255.0f )
							{
								gLog( "\t\tValue at %i,%i (%i) is %f (!= 255.0f)\n", i, j, i + j * 64, Selection->data.mapchunk->amap[layer][i + j * 64] );
								used = true;
							}

					gLog( "\t\t%s\n", used?"has something painted on it.":"should be somewhere below and has nothing on it." ) ;
					if( !used )
					{
						int l = layer;
						while( Selection->data.mapchunk->nTextures > l + 1 )
						{
							Selection->data.mapchunk->textures[l] = Selection->data.mapchunk->textures[l + 1];
							Selection->data.mapchunk->animated[l] = Selection->data.mapchunk->animated[l + 1];
							Selection->data.mapchunk->texFlags[l] = Selection->data.mapchunk->texFlags[l + 1];
							Selection->data.mapchunk->effectID[l] = Selection->data.mapchunk->effectID[l + 1];
							memcpy( Selection->data.mapchunk->amap[l], Selection->data.mapchunk->amap[l + 1], 64 * 64 );
							l++;
						}
						layer--;
						Selection->data.mapchunk->nTextures--;
					}
				}
			}
		}
#endif
		
		// fog distance or brush radius
		if( e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS ) 
			if( Environment::getInstance()->AltDown )
			{
				switch( terrainMode )
				{
				case 0:
					groundBrushRadius += 0.01f;
					if( groundBrushRadius > 1000.0f )
						groundBrushRadius = 1000.0f;
					else if( groundBrushRadius < 0.0005f )
						groundBrushRadius = 0.0005f;
					ground_brush_radius->setValue( groundBrushRadius / 1000 );
					break;
				case 1:
					blurBrushRadius += 0.01f;
					if( blurBrushRadius > 1000.0f )
						blurBrushRadius = 1000.0f;
					else if( blurBrushRadius < 0.01f )
						blurBrushRadius = 0.01f;
					blur_brush->setValue( blurBrushRadius / 1000 );
					break;
				case 2:
					textureBrush.setRadius( textureBrush.getRadius() + 0.1f );
					if( textureBrush.getRadius() > 100.0f )
						textureBrush.setRadius(100.0f);
					else if( textureBrush.getRadius() < 0.1f )
						textureBrush.setRadius(0.1f);
					paint_brush->setValue( textureBrush.getRadius() / 100 );
					break;
				}
			}
			else if( Environment::getInstance()->ShiftDown )
				gWorld->fogdistance += 60.0f;
			else
			{
				//change selected model sizesize
				keys=1;
			}

		if( e->keysym.sym == SDLK_KP_MINUS || e->keysym.sym == SDLK_MINUS ) 
			if( Environment::getInstance()->AltDown )
			{
				switch( terrainMode )
				{
				case 0:
					groundBrushRadius -= 0.01f;
					if( groundBrushRadius > 1000.0f )
						groundBrushRadius = 1000.0f;
					else if( groundBrushRadius < 0.0005f )
						groundBrushRadius = 0.0005f;
					ground_brush_radius->setValue( groundBrushRadius / 1000 );
					break;
				case 1:
					blurBrushRadius -= 0.01f;
					if( blurBrushRadius > 1000.0f )
						blurBrushRadius = 1000.0f;
					else if( blurBrushRadius < 0.01f )
						blurBrushRadius = 0.01f;
					blur_brush->setValue( blurBrushRadius / 1000 );
					break;
				case 2:
					textureBrush.setRadius( textureBrush.getRadius() - 0.1f );
					if( textureBrush.getRadius() > 100.0f )
						textureBrush.setRadius(100.0f);
					else if( textureBrush.getRadius() < 0.1f )
						textureBrush.setRadius(0.1f);
					paint_brush->setValue( textureBrush.getRadius() / 100 );
					break;
				}
			}
			else if( Environment::getInstance()->ShiftDown )
				gWorld->fogdistance -= 60.0f;
			else
			{
				//change selected model sizesize
				keys=-1;
			}

		// minimap
		if( e->keysym.sym == SDLK_m )
			if( mViewMode == eViewMode_Minimap )
				mViewMode = eViewMode_3D;
			else
				mViewMode = eViewMode_Minimap;

		// toogle between smooth / flat / linear
		if( e->keysym.sym == SDLK_y )
			switch( terrainMode )
			{
			case 0:
				groundBrushType++;
				groundBrushType = groundBrushType % 6;
				gGroundToggleGroup->Activate( groundBrushType );
				break;

			case 1:
				blurBrushType++;
				blurBrushType = blurBrushType % 3;
				gBlurToggleGroup->Activate( blurBrushType );
				break;
			}

		// is not used somewere else!!
		//! \todo	what is this?
		if( e->keysym.sym == SDLK_g )
			drawFlags = !drawFlags;

		// toogle tile mode
		if( e->keysym.sym == SDLK_u )
		{
			if( mViewMode == eViewMode_2D )
				mViewMode = eViewMode_3D;
			else
				mViewMode = eViewMode_2D;
			gWorld->ResetSelection();
		}

				// doodads set
		//! \todo	Does anyone use these?
		if( e->keysym.sym >= SDLK_0 && e->keysym.sym <= SDLK_9 && gWorld->IsSelection( eEntry_WMO ) )
			gWorld->GetCurrentSelection()->data.wmo->doodadset = e->keysym.sym - SDLK_0;
		
		// add a new bookmark
		if( e->keysym.sym == SDLK_F5 ) 
		{
			std::ofstream f( "bookmarks.txt", std::ios_base::app );
			f << gWorld->basename << " " << gWorld->camera.x << " " << gWorld->camera.y << " " << gWorld->camera.z << " " << ah << " " << av << " " << gWorld->getAreaID() << std::endl;
			f.close();
		}
	}
	else 
	{
		// key UP
		if( e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT )
			Environment::getInstance()->ShiftDown = false;

		if( e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT )
			Environment::getInstance()->AltDown = false;

		if( e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL )
			Environment::getInstance()->CtrlDown = false;

		// movement
		if( e->keysym.sym == SDLK_w && moving > 0.0f ) 
			moving = 0.0f;

		if( e->keysym.sym == SDLK_s && moving < 0.0f ) 
			moving = 0.0f;

		if( e->keysym.sym == SDLK_d && strafing > 0.0f ) 
			strafing = 0.0f;

		if( e->keysym.sym == SDLK_a && strafing < 0.0f ) 
			strafing = 0.0f;

		if( e->keysym.sym == SDLK_q && updown > 0.0f ) 
			updown = 0.0f;

		if( e->keysym.sym == SDLK_e && updown < 0.0f ) 
			updown = 0.0f;

		if( e->keysym.sym == SDLK_KP8 ) 
			keyx = 0;

		if( e->keysym.sym == SDLK_KP2 ) 
			keyx = 0;

		if( e->keysym.sym == SDLK_KP6 ) 
			keyz = 0;

		if( e->keysym.sym == SDLK_KP4 ) 
			keyz = 0;

		if( e->keysym.sym == SDLK_KP1 ) 
			keyy = 0;

		if( e->keysym.sym == SDLK_KP3 ) 
			keyy = 0;

		if( e->keysym.sym == SDLK_KP7 ) 
			keyr = 0;

		if( e->keysym.sym == SDLK_KP9 ) 
			keyr = 0;

		if( e->keysym.sym == SDLK_KP_MINUS || e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS) 
			keys = 0;

	}
}

void MapView::mousemove( SDL_MouseMotionEvent *e )
{
	if( !( SDL_GetAppState() & SDL_APPINPUTFOCUS ) )
		return;									// finally stop getting keys from chatting ..

	if ( ( look && !( Environment::getInstance()->ShiftDown || Environment::getInstance()->CtrlDown || Environment::getInstance()->AltDown ) ) || video.fullscreen ) 
	{
		ah += e->xrel / XSENS;
		av += mousedir * e->yrel / YSENS;
		if( av < -80.0f ) 
			av = -80.0f;
		else if ( av > 80.0f ) 
			av = 80.0f;
	}
	
	if( MoveObj )
	{
		mh = -video.ratio*e->xrel / float( video.xres );
		mv = -e->yrel / float( video.yres );
	}
	else
	{
		mh = 0.0f;
		mv = 0.0f;
	}

	if( Environment::getInstance()->ShiftDown || Environment::getInstance()->CtrlDown || Environment::getInstance()->AltDown )
	{
		rh = e->xrel / XSENS * 5.0f;
		rv = e->yrel / YSENS * 5.0f;
	}

	if( leftMouse && Environment::getInstance()->AltDown )
	{
		switch( terrainMode )
		{// das ist zum umstellen der brush größe
		case 0:
			groundBrushRadius += e->xrel / XSENS;
			if( groundBrushRadius > 1000.0f )
				groundBrushRadius = 1000.0f;
			else if( groundBrushRadius < 0.01f )
				groundBrushRadius = 0.01f;
			ground_brush_radius->setValue( groundBrushRadius / 1000 );
			break;
		case 1:
			blurBrushRadius += e->xrel / XSENS;
			if( blurBrushRadius > 1000.0f )
				blurBrushRadius = 1000.0f;
			else if( blurBrushRadius < 0.01f )
				blurBrushRadius = 0.01f;
			blur_brush->setValue( blurBrushRadius / 1000 );
			break;
		case 2:
			textureBrush.setRadius( textureBrush.getRadius() + e->xrel / XSENS );
			if( textureBrush.getRadius() > 100.0f )
				textureBrush.setRadius(100.0f);
			else if( textureBrush.getRadius() < 0.1f )
				textureBrush.setRadius(0.1f);
			paint_brush->setValue( textureBrush.getRadius() / 100.0f );
			break;
		}
	}

	if( leftMouse && LastClicked )
		LastClicked->processLeftDrag( e->x - 4, e->y - 4, e->xrel, e->yrel );

	if( mViewMode == eViewMode_3D && TestSelection )
	{
		if( !Environment::getInstance()->AutoSelecting )
			doSelection( 1 );
//		if( gWorld->IsSelection( eEntry_Model ) )
//			CurSelection = gWorld->GetCurrentSelection();
//		else 
//			CurSelection = 0;
	}	
	
	MouseX = e->x;
	MouseY = e->y;
}

void MapView::mouseclick( SDL_MouseButtonEvent *e )
{
	if( !( SDL_GetAppState() & SDL_APPINPUTFOCUS ) )
		return;									// finally stop getting keys from chatting ..

	if( e->type == SDL_MOUSEBUTTONDOWN ) 
	{
		switch( e->button )
		{
		case SDL_BUTTON_LEFT:
			LastClicked = tileFrames->processLeftClick( float( MouseX ), float( MouseY ) );
			leftMouse = true;	

			if( mViewMode == eViewMode_3D && !LastClicked )
				doSelection( 1 );

			break;

		case SDL_BUTTON_RIGHT:
			rightMouse = true;
			if( mViewMode == eViewMode_Help )
				mViewMode = eViewMode_3D; // Steff: exit help window when open
			look = true;
			break;

		case SDL_BUTTON_MIDDLE:
			if( gWorld->HasSelection() )
				MoveObj = true;
			break;
		}
	} 
	else if( e->type == SDL_MOUSEBUTTONUP ) 
	{
		switch( e->button )
		{
		case SDL_BUTTON_LEFT:
			leftMouse = false;
			if( LastClicked )
				LastClicked->processUnclick();
			if( !gWorld->HasSelection() || ( !gWorld->IsSelection( eEntry_Model ) && !gWorld->IsSelection( eEntry_WMO ) ) ) 
				Environment::getInstance()->AutoSelecting = true;
			break;

		case SDL_BUTTON_RIGHT:
			rightMouse = false;
			look = false;
			break;

		case SDL_BUTTON_MIDDLE:
			MoveObj = false;
			break;
		}
	}

	// check menu settings and switch hole mode
	if(terrainMode!=3)
	{
		if(Settings::getInstance()->holelinesOn) 
			Environment::getInstance()->view_holelines = true;
		else Environment::getInstance()->view_holelines = false;
	}
}


