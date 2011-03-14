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
#include "misc.h"
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
#include "ui_ZoneIdBrowser.h" // Gui
#include "ToggleGroup.h" // ToggleGroup
#include "textUI.h" // textUI
#include "gradient.h" // gradient
#include "checkboxUI.h" // checkboxUI
#include "Toolbar.h" // Toolbar
#include "ToolbarIcon.h" // ToolbarIcon
#include "textureUI.h" // textureUI
#include "statusBar.h" // statusBar
#include "detailInfos.h" // detailInfos
#include "menuBar.h" // menuBar, menu items, ..
#include "minimapWindowUI.h"

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

Gui *mainGuiPointer;
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
int saveterrainMode = 0;

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
ToggleGroup * gFlagsToggleGroup;

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
	if( pMode == 1 )
		gWorld->reloadTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );
	else if( pMode == 0 )
		gWorld->saveTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );
	else if( pMode == 2 )
		gWorld->saveChanged();
}

void change_settings_window(int oldid, int newid)
{
  if(!setting_ground || !setting_blur || !settings_paint)
    return;
	setting_ground->hidden=true;
	setting_blur->hidden=true;
	settings_paint->hidden=true;
	mainGuiPointer->ZoneIDBrowser->hidden = true;
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
	case 5:
		mainGuiPointer->ZoneIDBrowser->hidden=false;
	break;
	}
}

//! \todo	Do this nicer?
void openHelp( frame* /*button*/, int /*id*/ )
{
	static_cast<MapView*>( gStates.back() )->ViewHelp();
}

void closeHelp( frame* /*button*/, int /*id*/ )
{
	static_cast<MapView*>( gStates.back() )->View3D();
}

void ResetSelectedObjectRotation( frame* /*button*/, int /*id*/ )
{
	if( gWorld->IsSelection( eEntry_WMO ) )
	{
		gWorld->GetCurrentSelection()->data.wmo->resetDirection();
		gWorld->setChanged(gWorld->GetCurrentSelection()->data.wmo->pos.x, gWorld->GetCurrentSelection()->data.wmo->pos.z);
	}
	else if( gWorld->IsSelection( eEntry_Model ) )
	{
		gWorld->GetCurrentSelection()->data.model->resetDirection();
		gWorld->setChanged(gWorld->GetCurrentSelection()->data.model->pos.x, gWorld->GetCurrentSelection()->data.model->pos.z);
	}
}

void SnapSelectedObjectToGround( frame* /*button*/, int /*id*/ )
{
	if( gWorld->IsSelection( eEntry_WMO ) )
	{
		Vec3D t = Vec3D( gWorld->GetCurrentSelection()->data.wmo->pos.x, gWorld->GetCurrentSelection()->data.wmo->pos.z, 0 );
		gWorld->GetVertex( gWorld->GetCurrentSelection()->data.wmo->pos.x, gWorld->GetCurrentSelection()->data.wmo->pos.z, &t );
		gWorld->GetCurrentSelection()->data.wmo->pos = t;
		gWorld->setChanged(gWorld->GetCurrentSelection()->data.wmo->pos.x, gWorld->GetCurrentSelection()->data.wmo->pos.z);

	}
	else if( gWorld->IsSelection( eEntry_Model ) )
	{
		Vec3D t = Vec3D( gWorld->GetCurrentSelection()->data.model->pos.x, gWorld->GetCurrentSelection()->data.model->pos.z, 0 );
		gWorld->GetVertex( gWorld->GetCurrentSelection()->data.model->pos.x, gWorld->GetCurrentSelection()->data.model->pos.z, &t );
		gWorld->GetCurrentSelection()->data.model->pos = t;		
		gWorld->setChanged(gWorld->GetCurrentSelection()->data.model->pos.x, gWorld->GetCurrentSelection()->data.model->pos.z);
	}
}

void CopySelectedObject( frame* /*button*/, int /*id*/ )
{
	//! \todo	copy selected object path to clipboard
	if( gWorld->HasSelection() )
	{
		Environment::getInstance()->set_clipboard( gWorld->GetCurrentSelection() );
	}
}

void PasteSelectedObject( frame* /*button*/, int /*id*/ )
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

void DeleteSelectedObject( frame* /*button*/, int /*id*/ )
{
	if( gWorld->IsSelection( eEntry_WMO ) )
		gWorld->deleteWMOInstance( gWorld->GetCurrentSelection()->data.wmo->mUniqueID );
	else if( gWorld->IsSelection( eEntry_Model ) )
		gWorld->deleteModelInstance( gWorld->GetCurrentSelection()->data.model->d1 );
}

void InsertObject( frame* /*button*/, int id )
{
  //! \todo Beautify.
  
	// ID switch the import way


	// Test if there is an selection
	if( !gWorld->HasSelection() )
		return;
	// the list of the models to import
	std::vector<std::string> m2s_to_add;
	std::vector<std::string> wmos_to_add;

	// the import file
	std::string importFile;
	
	const char* filesToAdd[15] = {"","","World\\Scale\\humanmalescale.m2","World\\Scale\\50x50.m2","World\\Scale\\100x100.m2","World\\Scale\\250x250.m2","World\\Scale\\500x500.m2","World\\Scale\\1000x1000.m2","World\\Scale\\50yardradiusdisc.m2","World\\Scale\\200yardradiusdisc.m2","World\\Scale\\777yardradiusdisc.m2","World\\Scale\\50yardradiussphere.m2","World\\Scale\\200yardradiussphere.m2","World\\Scale\\777yardradiussphere.m2",""};

	// MODELINSERT FROM TEXTFILE
	// is a source file set in config file?
	
	switch(id)
		{
			 case 0:
			 case 14:
				if( FileExists( "noggIt.conf" ) )
				{
					ConfigFile config( "noggIt.conf" );
					config.readInto( importFile, "ImportFile" );
				}
			 break;
 
			 case 1:
				importFile="Import.txt"; //	use import.txt in noggit folder!
			 break;
			 
			 default:
				m2s_to_add.push_back( filesToAdd[id] );
				break;
	} 


	std::string lastModel;
	int lastTyp =0;

	if(importFile!="")
	{
		size_t foundString;
		std::string line;
		std::string findThis;
		std::ifstream fileReader(importFile.c_str());
		if (fileReader.is_open())
		{
			while (! fileReader.eof() )
			{
				getline (fileReader,line);
				if(line.find(".m2")!= std::string::npos || line.find(".M2")!= std::string::npos || line.find(".MDX")!= std::string::npos || line.find(".mdx")!= std::string::npos )
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

					// swap mdx to m2
					size_t found = line.rfind( ".mdx" );
					if( found != std::string::npos )
						line.replace( found, 4, ".m2" );
					found = line.rfind( ".MDX" );
					if( found != std::string::npos )
						line.replace( found, 4, ".m2" );

					m2s_to_add.push_back( line );
					lastModel = line;
					lastTyp=1;
				}
				else if(line.find(".wmo")!= std::string::npos || line.find(".WMO")!= std::string::npos )
				{
					// WMO inside line
					findThis = "Loading WMO ";
					foundString = line.find(findThis);
					// is it the modelviewer log then cut the log messages out
					if(foundString != std::string::npos)
					{
						// cut path
						line = line.substr( foundString+findThis.size() );
					}
					wmos_to_add.push_back(line);
					lastModel = line;
					lastTyp=2;
				}
			}
			fileReader.close();
		}
		else 
		{
			// file not exist, no rights ore other error
			LogError << importFile << std::endl;
		}
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

	if(id==14)
	{
		// import only last model from viewer
		if(lastTyp==1)
		{
			//m2
			if( !MPQFile::exists(lastModel) )
				LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
			else
				gWorld->addM2( static_cast<Model*>(ModelManager::items[ModelManager::add(lastModel)]), selectionPosition );
		}
		else if(lastTyp==2)
		{
			//wmo	
			if( !MPQFile::exists(lastModel) )
				LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
			else
				gWorld->addWMO( static_cast<WMO*>(WMOManager::items[WMOManager::add(lastModel)]), selectionPosition );
		}
	}
	else
	{
		for( std::vector<std::string>::iterator it = wmos_to_add.begin(); it != wmos_to_add.end(); ++it )
		{
		
			if( !MPQFile::exists(*it) )
			{
				LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
				continue;
			}
		
			gWorld->addWMO( static_cast<WMO*>(WMOManager::items[WMOManager::add(*it)]), selectionPosition );
		}

		for( std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it )
		{

			if( !MPQFile::exists(*it) )
			{
				LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
				continue;
			}

			gWorld->addM2( static_cast<Model*>(ModelManager::items[ModelManager::add(*it)]), selectionPosition );
		}
	}
	//! \todo Memoryleak: These models will never get deleted.
}

void view_texture_palette( frame* /*button*/, int /*id*/ )
{
	TexturePalette->hidden = !TexturePalette->hidden;
}

void exit_tilemode(  frame* /*button*/, int /*id*/ )
{
	gPop = true;
}

void test_menu_action(  frame* /*button*/, int id )
{
	if(id == 1)
		gWorld->setAreaID(5000,misc::FtoIround((gWorld->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((gWorld->camera.z-(TILESIZE/2))/TILESIZE));
}

void changeZoneIDValue(frame *f,int set)
{
	Environment::getInstance()->selectedAreaID = set;
	if( Environment::getInstance()->areaIDColors.find(set) == Environment::getInstance()->areaIDColors.end() )
	{
		Vec3D newColor = Vec3D( misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) );
		Environment::getInstance()->areaIDColors.insert( std::pair<int,Vec3D>(set, newColor) );
	}
}

MapView::MapView(float ah0, float av0): ah(ah0), av(av0), mTimespeed( 0.0f )
{
	LastClicked=0;

	moving = strafing = updown = 0.0f;

	mousedir = -1.0f;

	movespd = SPEED;

	lastBrushUpdate = 0;
	textureBrush.init();

	look = false;
	hud = true;
	set_areaid = false;
	mViewMode = eViewMode_3D;

	// create main gui object that holds all other gui elements for access ( in the future ;) )
	mainGui = new Gui( this );
	mainGuiPointer = mainGui;
	mainGui->guiToolbar->current_texture->setClickFunc( view_texture_palette, 0 );

	mainGui->ZoneIDBrowser->setMapID( gWorld->getMapID() );
	mainGui->ZoneIDBrowser->setChangeFunc( changeZoneIDValue );
	tool_settings_x = video.xres - 186;
	tool_settings_y = 38;
	
	// Raise/Lower
	setting_ground=new window( tool_settings_x, tool_settings_y, 180.0f, 160.0f );
	setting_ground->movable = true;
	mainGui->tileFrames->addChild( setting_ground );

	setting_ground->addChild( new textUI( 78.5f, 2.0f, "Raise / Lower", arial14, eJustifyCenter ) );
	
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
	mainGui->tileFrames->addChild(setting_blur);

	setting_blur->addChild( new textUI( 78.5f, 2.0f, "Flatten / Blur", arial14, eJustifyCenter ) );

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

	mainGui->tileFrames->addChild(settings_paint);

	settings_paint->addChild( new textUI( 78.5f, 2.0f, "3D Paint", arial14, eJustifyCenter ) );
	
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

	mainGui->tileFrames->addChild(TexturePalette = TexturingUI::createTexturePalette(4,8,mainGui));
	TexturePalette->hidden=true;
	mainGui->tileFrames->addChild(SelectedTexture = TexturingUI::createSelectedTexture());
	SelectedTexture->hidden=true;
	mainGui->tileFrames->addChild(TexturingUI::createTilesetLoader());
	mainGui->tileFrames->addChild(TexturingUI::createTextureFilter());
	mainGui->tileFrames->addChild(MapChunkWindow = TexturingUI::createMapChunkWindow());
	MapChunkWindow->hidden=true;
	
	// create the menu
	menuBar * mbar = new menuBar();

	mbar->AddMenu( "File" );
	mbar->AddMenu( "Edit" );
	mbar->AddMenu( "View" );
	mbar->AddMenu( "Assist" );
	mbar->AddMenu( "Help" );

	mbar->GetMenu( "File" )->AddMenuItemButton( "CTRL + SHIFT + S Save current tile", SaveOrReload, 0 );
	mbar->GetMenu( "File" )->AddMenuItemButton( "CTRL + S Save all", SaveOrReload, 2 );
	mbar->GetMenu( "File" )->AddMenuItemButton( "SHIFT + J Reload current tile", SaveOrReload, 1 );
	
	//mbar->GetMenu( "File" )->AddMenuItemSeperator( "Test" );
	//mbar->GetMenu( "File" )->AddMenuItemButton( "AreaID", test_menu_action, 1 );

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

	//mbar->GetMenu( "Edit" )->AddMenuItemSeperator( "Modify current ADT" );

	mbar->GetMenu( "Assist" )->AddMenuItemSeperator( "Add model" );
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "all from ModelViewer", InsertObject, 0	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "last from ModelViewer", InsertObject, 14	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "from Text File", InsertObject, 1	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Human scale", InsertObject, 2	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Cube 50", InsertObject, 3	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Cube 100", InsertObject, 4	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Cube 250", InsertObject, 5	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Cube 500", InsertObject, 6	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Cube 1000", InsertObject, 7	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Disc 50", InsertObject, 8	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Disc 200", InsertObject, 9	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Disc 777", InsertObject, 10	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Sphere 50", InsertObject, 11	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Sphere 200", InsertObject, 12	);
	mbar->GetMenu( "Assist" )->AddMenuItemButton( "Sphere 777", InsertObject, 13	);
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

	mainGui->tileFrames->addChild( mbar );
}

MapView::~MapView()
{
	if( mainGui )
	{
		delete mainGui;
		mainGui = NULL;
		mainGuiPointer = NULL;
	}
	if( gWorld )
	{
		delete gWorld;
		gWorld = NULL;
	}
}

void MapView::tick( float t, float dt )
{	
  using std::min;
  dt = min( dt, 1.0f );
		
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
			// Set move scale and rotate for numpad keys
			if(Environment::getInstance()->CtrlDown && Environment::getInstance()->ShiftDown) moveratio=0.05f;
			else if(Environment::getInstance()->ShiftDown) moveratio=0.2f;
			else if(Environment::getInstance()->CtrlDown) moveratio=0.3f;
			else moveratio=0.1f;

			if( keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0) 
			{
				// Move scale and rotate with numpad keys
				if( Selection->type == eEntry_WMO )
				{	
					gWorld->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
					Selection->data.wmo->pos.x += keyx * moveratio;
					Selection->data.wmo->pos.y += keyy * moveratio;
					Selection->data.wmo->pos.z += keyz * moveratio;
					Selection->data.wmo->dir.y += keyr * moveratio * 2;
					gWorld->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
				}

				if( Selection->type == eEntry_Model )
				{
					gWorld->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
					Selection->data.model->pos.x += keyx * moveratio;
					Selection->data.model->pos.y += keyy * moveratio;
					Selection->data.model->pos.z += keyz * moveratio;
					Selection->data.model->dir.y += keyr * moveratio * 2;
					Selection->data.model->sc += keys * moveratio / 50;
					gWorld->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
				}
			}

	    Vec3D ObjPos;
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
					 gWorld->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z); // before move
					 ObjPos.x = 80.0f;
					 Selection->data.wmo->pos+=mv * dirUp * ObjPos.x;
					 Selection->data.wmo->pos-=mh * dirRight * ObjPos.x;
					 Selection->data.wmo->extents[0] = Selection->data.wmo->pos - Vec3D(1,1,1);
					 Selection->data.wmo->extents[1] = Selection->data.wmo->pos + Vec3D(1,1,1);
					 gWorld->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z); // after move. If moved to another ADT
				}
				else if( Selection->type == eEntry_Model )
					if( Environment::getInstance()->AltDown )
					{
						gWorld->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
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
						gWorld->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z); // before move
						ObjPos.x = 80.0f;
						Selection->data.model->pos += mv * dirUp * ObjPos.x;
						Selection->data.model->pos -= mh * dirRight * ObjPos.x;
						gWorld->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z); // after move. If moved to another ADT
					}
		

			// rotating objects
			if( look )
			{
				float * lTarget = NULL; 
				bool lModify = false;
				
				if( Selection->type == eEntry_Model )
				{
					gWorld->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
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
					gWorld->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
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
					{						
						// Move ground up
						gWorld->changeTerrain( xPos, zPos, 7.5f * dt * groundBrushSpeed, groundBrushRadius, groundBrushType );
					}
					else if( Environment::getInstance()->CtrlDown )
					{
						// Move ground down
						gWorld->changeTerrain( xPos, zPos, -7.5f * dt * groundBrushSpeed, groundBrushRadius, groundBrushType );
					}
						
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
						else if( TexturingUI::getSelectedTexture() )
						{
							if( textureBrush.needUpdate() )
    					{
    						textureBrush.GenerateTexture();
    					}
							
							gWorld->paintTexture( xPos, zPos, &textureBrush, brushLevel, 1.0f - pow( 1.0f - brushPressure, dt * 10.0f ), TextureManager::add( TexturingUI::getSelectedTexture()->name ) );
						}
					}
						
				break;
					
				case 3:
					if( Environment::getInstance()->ShiftDown	)
						gWorld->removeHole( xPos, zPos );
					else if( Environment::getInstance()->CtrlDown )
						gWorld->addHole( xPos, zPos );
						
				break;

				case 4:
					if( Environment::getInstance()->ShiftDown	)
					{
						// draw the selected AreaId on current selected chunk
						nameEntry * lSelection = gWorld->GetCurrentSelection();
						int mtx,mtz,mcx,mcy;
						mtx = lSelection->data.mapchunk->mt->mPositionX;
						mtz = lSelection->data.mapchunk->mt->mPositionZ ;
						mcx = lSelection->data.mapchunk->px;
						mcy = lSelection->data.mapchunk->py;
						gWorld->setAreaID( Environment::getInstance()->selectedAreaID, mtx,mtz, mcx, mcy );
					}
					else if( Environment::getInstance()->CtrlDown )
					{
						// pick areaID from chunk
						int newID = gWorld->GetCurrentSelection()->data.mapchunk->areaID;
						Environment::getInstance()->selectedAreaID = newID;
						mainGui->ZoneIDBrowser->setZoneID(newID);
					}

				break;

				case 5:
					if( Environment::getInstance()->ShiftDown	)
						gWorld->setFlag( true, xPos, zPos );
					else if( Environment::getInstance()->CtrlDown )
						gWorld->setFlag( false, xPos, zPos );
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
				
					float mX, mY;
					mX = CHUNKSIZE * 4.0f * video.ratio * ( float( MouseX ) / float( video.xres ) - 0.5f ) / gWorld->zoom+gWorld->camera.x;
					mY = CHUNKSIZE * 4.0f * ( float( MouseY ) / float( video.yres ) - 0.5f) / gWorld->zoom+gWorld->camera.z;

				if( Environment::getInstance()->CtrlDown )
					gWorld->eraseTextures( mX, mY );
				else
				{
					if( textureBrush.needUpdate() )
					{
						textureBrush.GenerateTexture();
					}
					
					if( Environment::getInstance()->ShiftDown )
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
	{
    textureBrush.GenerateTexture();
	}

	gWorld->time += this->mTimespeed * dt;
	gWorld->animtime += dt * 1000.0f;
	globalTime = static_cast<int>( gWorld->animtime );
	
	gWorld->tick(dt);

	if( !MapChunkWindow->hidden )
	{
		if( gWorld->GetCurrentSelection() && gWorld->GetCurrentSelection()->type == eEntry_MapChunk )
	    TexturingUI::setChunkWindow( gWorld->GetCurrentSelection()->data.mapchunk );
	}
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

void MapView::displayViewMode_Help( float /*t*/, float /*dt*/ )
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
	

	freetype::shprint( *arial16, 60.0f, 40.0f, 
	"Basic controles\n\n"					
		"Left mouse button moves the camera\n"
		"Mouse left click - select chunk or object\n"
		"Both mouse buttons - move forward\n"
		"I - invert mouse up and down\n"
		"Q,E - move up,down\n"
		"A,D,W,S - move left,right,forward,backward\n"
		"M - show minimap\n"
		"U - 2d texture editor\n"
		//"C - chunk settings\n" //! \todo: C chunk settings must get fixed first. Then turn on this again
		"H - help\n"
		"Shift + R - turn camera 180 degres\n"
		"Shift + F4 - change to auto select mode\n"
		"Esc - exit to main menu\n"
		"\n"
	"Toggles\n"	
		"\n"
		"F1 - toggle M2s\n"
		"F2 - toggle WMO doodads set\n" 
		"F3 - toggle ground\n"
		"F4 - toggle GUI\n"
		"F6 - toggle WMOs\n"
		"F7 - toggle chunk (red) and ADT (green) lines\n"
		"F8 - toggle detailed infotext\n"
		"F9 - toggle map contour\n"
		"F - toggle fog\n"
		"TAB - toggle UI view\n"
		"x - texture palette\n"
		"CTRL + x - detail window\n"
		"R/T - Move true the editing modes\n"
		"\n"
		"Files:\n"
		"F5 - save bookmark\n"
		"F10 - reload BLP\n"
		"F11 - reload M2s\n"
		"F12 - reload wmo\n"
		"Shift + J - reload ADT tile\n"
		"CTRL + S -  Save all changed ADT tiles\n"
		"CTRL + SHIFT + S - Save ADT tiles camera position\n"
	);
	
	freetype::shprint( *arial16, video.xres - 400.0f, 40.0f, 
		"Edit ground:\n"
		"Shift + F1 - toggle ground edit mode\n"
		"T - change terrain mode\n"
		"Y - changes brush type\n"	
		"ALT + left mouse + mouse move - change brush size\n"
		"Terrain mode - raise/lower\n"
		"Left mouse click + Shift - raise terrain\n"
		"Left mouse click + Alt - lower terrain\n"
		"Terrain mode - flatten/blur\n"
		"Left mouse click + Shift - flatten terrain\n"
		"Left mouse	click + Alt - blur terrain\n"	
		"Z - change the mode in the option window\n"
		"\n"
		"Edit objects if a model is selected with left click:\n"
		"Hold middle mouse - move object\n"
		"Hold middle mouse + Alt - scale M2\n"
		"Hold left mouse + Shift, Ctrl or Alt - rotate object\n"
		"0-9 - change doodads set of selected WMO\n"
		"Ctrl+R - Reset rotation\n"
		"PageDown - Set object to Groundlevel\n"
		"CTRL + C - Copy object to clipboard\n"
		"CTRL + V - Paste object on mouse position\n"
		"-/+ - scale M2\n"
		"Numpad 7/9 - rotate object\n"
		"Numpad 4/8/6/2 - vertical position\n"
		"Numpad 1/3 -  move up/dow\n"
		"With Shift double speed \n" 
		"With CTRL triple speed \n"
		"With Shift and CTRL together half speed \n"
		"\n"
		"Edit texture:\n"
		"Hold CTRL + Shift - clear all textures on chunk\n"
		"Hold CTRL - draw texture or fills if chunk is empty\n"
		"\n"
		"Adjust:\n"
		"O,P - slower/faster movement\n"
		"B,N - slower/faster time\n"
		"Shift -/+ - fog distance when no model is selected\n"
	);

}

void MapView::displayViewMode_Minimap( float /*t*/, float /*dt*/ )
{
	// not used now. !!!
	// swap with minimapWindow. Delete if minimpa window run 100 %
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

void MapView::displayViewMode_2D( float /*t*/, float /*dt*/ )
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
		tRadius=textureBrush.getRadius()/CHUNKSIZE;// *gWorld->zoom;
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
  	glColor4f(1.0f,1.0f,1.0f,1.0f);
  	glActiveTexture(GL_TEXTURE0);
  	glDisable(GL_TEXTURE_2D);
  	
  	mainGui->render( true );
  	
  	glActiveTexture(GL_TEXTURE0);
  	glEnable(GL_TEXTURE_2D);
  }
}

void MapView::displayViewMode_3D( float /*t*/, float /*dt*/ )
{
	if( Environment::getInstance()->AutoSelecting && Settings::getInstance()->AutoSelectingMode )
		doSelection( 0 );
	
	video.set3D();

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
		
		mainGui->render( false );
		
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
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
  mainGui->resize();
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
		{
			key_w = true;
			moving = 1.0f;
		}

		// save
		if( e->keysym.sym == SDLK_s )
			if( Environment::getInstance()->CtrlDown && Environment::getInstance()->ShiftDown )
				gWorld->saveTile( int( gWorld->camera.x ) / TILESIZE, int( gWorld->camera.z ) / TILESIZE );
			else if( Environment::getInstance()->CtrlDown)
				gWorld->saveChanged();
			else
				moving = -1.0f;
		
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


		// This was Bekets function to replace chunk textures. Redo.
		if (e->keysym.sym == SDLK_z && gWorld->IsSelection( eEntry_MapChunk) ) 
		{
			//setChunk(Selection->data.mapchunk);
		}

		// delete object
		if( e->keysym.sym == SDLK_DELETE )
			DeleteSelectedObject( 0, 0 );

		// open chunk settings window or copy & paste
		if( e->keysym.sym == SDLK_c)
		{	
			if( Environment::getInstance()->CtrlDown )
				CopySelectedObject( 0, 0 );
			/*else if( gWorld->IsSelection( eEntry_MapChunk ) )
			{
				TexturingUI::setChunkWindow( gWorld->GetCurrentSelection()->data.mapchunk );
				MapChunkWindow->hidden = false;
			}*/
		}
		if( e->keysym.sym == SDLK_v && Environment::getInstance()->CtrlDown )
			PasteSelectedObject( 0, 0 );

		if( e->keysym.sym == SDLK_x )
		{
			if( Environment::getInstance()->CtrlDown )
			{
				// toggle detail window
				mainGui->guidetailInfos->hidden = !mainGui->guidetailInfos->hidden;
			}
			else
			{
				// toggle texture window
				view_texture_palette( 0, 0 );
			}


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
		{
			if( Environment::getInstance()->CtrlDown )
				ResetSelectedObjectRotation( 0, 0 );
			else if( Environment::getInstance()->ShiftDown )
				ah += 180.0f;
			else
			{
		    terrainMode = ( terrainMode + 1 ) % 6;
		    
			  // Set the right icon in toolbar
			  mainGui->guiToolbar->IconSelect( terrainMode );				
			}
		}		
		
		// toggle editing mode
		if( e->keysym.sym == SDLK_t ) 
		{
		  //! \todo Modulo.
		  terrainMode =  terrainMode - 1;
		  if(terrainMode<0) terrainMode = 5;
			// Set the right icon in toolbar
			mainGui->guiToolbar->IconSelect( terrainMode );
		}

		// clip object to ground
		if( e->keysym.sym == SDLK_PAGEDOWN )
			SnapSelectedObjectToGround( 0, 0 );

		// speed of daytime.
		if( e->keysym.sym == SDLK_n )
			this->mTimespeed += 90.0f;
		
		if( e->keysym.sym == SDLK_b )
			this->mTimespeed -= 90.0f;
		
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
		
		// toggle draw water
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
			else if( Environment::getInstance()->ShiftDown && ( !gWorld->HasSelection() || ( gWorld->HasSelection() && gWorld->GetCurrentSelection()->type == eEntry_MapChunk) )  )
				gWorld->fogdistance += 60.0f;// fog change only when no model is selected!
			else
			{
				//change selected model size
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
			else if( Environment::getInstance()->ShiftDown && ( !gWorld->HasSelection() || ( gWorld->HasSelection() && gWorld->GetCurrentSelection()->type == eEntry_MapChunk) )  )
				gWorld->fogdistance -= 60.0f; // fog change only when no model is selected!
			else
			{
				//change selected model sizesize
				keys=-1;
			}

		// minimap
		if( e->keysym.sym == SDLK_m )
			mainGui->minimapWindow->hidden = !mainGui->minimapWindow->hidden;

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
			{
				mViewMode = eViewMode_3D;
				terrainMode = saveterrainMode;
				// Set the right icon in toolbar
				mainGui->guiToolbar->IconSelect( terrainMode );	
			}
			else
			{
				mViewMode = eViewMode_2D;
				saveterrainMode = terrainMode;
				terrainMode = 2;
				// Set the right icon in toolbar
				mainGui->guiToolbar->IconSelect( terrainMode );	
			}

		}

				// doodads set
		//! \todo	Does anyone use these?
		//! Yes to change the doodadset of houses i use it . Steff :)
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
		if( e->keysym.sym == SDLK_w) 
		{
			key_w = false;
			if( !(leftMouse && rightMouse) && moving > 0.0f) moving = 0.0f;
		}


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

		mainGui->minimapWindow->changePlayerLookAt(ah);
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
		{	// das ist zum umstellen der brush größe
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
	{
		LastClicked->processLeftDrag( e->x - 4, e->y - 4, e->xrel, e->yrel );
	}

	if( mViewMode == eViewMode_3D && TestSelection )
	{
		if( !Environment::getInstance()->AutoSelecting )
		{
			doSelection( 1 );
		}
	}	
	
	if( mViewMode == eViewMode_2D && leftMouse && !( Environment::getInstance()->ShiftDown || Environment::getInstance()->CtrlDown || Environment::getInstance()->AltDown )  )
	{
		strafing = ((e->xrel / XSENS) / -1) * 5.0f;
		moving = (e->yrel / YSENS) * 5.0f;
	}	

	if( mViewMode == eViewMode_2D && rightMouse && !( Environment::getInstance()->ShiftDown || Environment::getInstance()->CtrlDown || Environment::getInstance()->AltDown )  )
	{
		updown = (e->yrel / YSENS);
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
				leftMouse = true;	
			break;

			case SDL_BUTTON_RIGHT:
				rightMouse = true;
			break;

			case SDL_BUTTON_MIDDLE:
				if( gWorld->HasSelection() )
					MoveObj = true;
			break;
		}

		 if (leftMouse && rightMouse)
		 {
			 // Both buttons
			 moving = 1.0f;
		 }
		 else if (leftMouse)
		 {
			// Only left
			LastClicked = mainGui->tileFrames->processLeftClick( float( MouseX ), float( MouseY ) );	
			if( mViewMode == eViewMode_3D && !LastClicked )
  		{
  			doSelection( 1 );
  		}
    }
    else if (rightMouse)
    {
      // Only right
      if( mViewMode == eViewMode_Help )
      	mViewMode = eViewMode_3D; // Steff: exit help window when open
      look = true;
    }
	} 
	else if( e->type == SDL_MOUSEBUTTONUP ) 
	{
		switch( e->button )
		{
			case SDL_BUTTON_LEFT:
				leftMouse = false;
			break;

			case SDL_BUTTON_RIGHT:
				rightMouse = false;
			break;

			case SDL_BUTTON_MIDDLE:
				MoveObj = false;
			break;
		}

		 if (!leftMouse)
		 {
			//  left
			if( LastClicked )
				LastClicked->processUnclick();
			if( !gWorld->HasSelection() || ( !gWorld->IsSelection( eEntry_Model ) && !gWorld->IsSelection( eEntry_WMO ) ) ) 
				Environment::getInstance()->AutoSelecting = true;
			if(!key_w && moving > 0.0f )moving = 0.0f;
			
			if( mViewMode == eViewMode_2D )
			{
				strafing = 0;
				moving = 0;
			}
		 }
		 
		 if (!rightMouse)
		 {
			//  right
			if( mViewMode == eViewMode_Help )
				mViewMode = eViewMode_3D; // Steff: exit help window when open
			look = false;
			if(!key_w && moving > 0.0f )moving = 0.0f;

			if( mViewMode == eViewMode_2D )
			{
				updown = 0;
			}
		 }
	}

	// check menu settings and switch hole mode
	//! \todo why the hell is this here?
	if(terrainMode!=3)
	{
		Environment::getInstance()->view_holelines = Settings::getInstance()->holelinesOn;
	}
}
