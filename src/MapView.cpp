#undef _UNICODE

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <vector>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenuBar>
#include <QSettings>

#include <boost/filesystem.hpp>

#ifdef __FILESAREMISSING
#include <IL/il.h>
#endif

#include "Brush.h" // brush
#include "ConfigFile.h"
#include "DBC.h"
#include "Environment.h"
#include "FreeType.h" // freetype::
#include "Log.h"
#include "MapChunk.h"
#include "MapView.h"
#include "Misc.h"
#include "ModelManager.h" // ModelManager
#include "Noggit.h" // gStates, gPop, gFPS, arial14, morpheus40, arial...
#include "Project.h"
#include "Settings.h"
#include "Environment.h"
#include "TextureManager.h" // TextureManager, Texture
#include "UIAppInfo.h" // appInfo
#include "UICheckBox.h" // UICheckBox
#include "UICursorSwitcher.h" // UICursorSwitcher
#include "UIDetailInfos.h" // detailInfos
#include <UIDoodadSpawner.h>
#include "UIGradient.h" // UIGradient
#include "UIMapViewGUI.h" // UIMapViewGUI
#include "UIMenuBar.h" // UIMenuBar, menu items, ..
#include "UIMinimapWindow.h" // UIMinimapWindow
#include "UISlider.h" // UISlider
#include "UIStatusBar.h" // statusBar
#include "UIText.h" // UIText
#include "UITexture.h" // textureUI
#include "UITexturePicker.h"
#include "UITextureSwitcher.h"
#include "UITexturingGUI.h"
#include "UIToggleGroup.h" // UIToggleGroup
#include "UIToolbar.h" // UIToolbar
#include "UIToolbarIcon.h" // ToolbarIcon
#include "UIZoneIDBrowser.h"
#include "WMOInstance.h" // WMOInstance
#include "World.h"

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

extern nameEntryManager SelectionNames;

// extern row and col form Palette UI


// This variables store the current status of the
// Shift, Alt and CTRL keys


bool  leftMouse=false;
bool  leftClicked=false;
bool  rightMouse=false;
bool  painting=false;

// Vars for the ground editing toggle mode
// store the status of some view settings when
// the ground editing mode is switched on
// to restore them if switch back again

bool  alloff = true;
bool  alloff_models = false;
bool  alloff_doodads = false;
bool  alloff_contour = false;
bool  alloff_wmo = false;
bool  alloff_detailselect = false;
bool  alloff_fog = false;
bool  alloff_terrain = false;

UISlider* ground_brush_radius;
float groundBrushRadius=15.0f;
UISlider* ground_brush_speed;
float groundBrushSpeed=1.0f;
int    groundBrushType=2;

UISlider* blur_brush;
float blurBrushRadius=10.0f;
int    blurBrushType=2;

UISlider* paint_brush;

float brushPressure=0.9f;
float brushLevel=255.0f;

int terrainMode=0;
int saveterrainMode = 0;

brush textureBrush;


UICursorSwitcher* CursorSwitcher;

bool Saving=false;

UIFrame* LastClicked;


// main GUI object
UIMapViewGUI* mainGui;

UIFrame* MapChunkWindow;

UIToggleGroup * gBlurToggleGroup;
UIToggleGroup * gGroundToggleGroup;
UIToggleGroup * gFlagsToggleGroup;

UIWindow *setting_ground;
UIWindow *setting_blur;
UIWindow *settings_paint;


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

void change_settings_window(int oldid, int newid)
{
  if(!setting_ground || !setting_blur || !settings_paint)
    return;
  setting_ground->hide();
  setting_blur->hide();
  settings_paint->hide();
  if(!mainGui || !mainGui->TexturePalette)
    return;
  mainGui->TexturePalette->hide();
  // fetch old win position
  switch(oldid)
  {
  case 1:
    tool_settings_x=setting_ground->x();
    tool_settings_y=setting_ground->y();
  break;
  case 2:
    tool_settings_x=setting_blur->x();
    tool_settings_y=setting_blur->y();
  break;
  case 3:
    tool_settings_x=settings_paint->x();
    tool_settings_y=settings_paint->y();
  break;
  }
  // set new win pos and make visible
  switch(newid)
  {
  case 1:
    setting_ground->x( tool_settings_x );
    setting_ground->y( tool_settings_y );
    setting_ground->show();
  break;
  case 2:
    setting_blur->x( tool_settings_x );
    setting_blur->y( tool_settings_y );
    setting_blur->show();
  break;
  case 3:
    settings_paint->x( tool_settings_x );
    settings_paint->y( tool_settings_y );
    settings_paint->show();
  break;
  }
}

void openSwapper( UIFrame*, int )
{
  mainGui->TextureSwitcher->setPosition(settings_paint->x() , settings_paint->y()) ;
  mainGui->TextureSwitcher->show();
  settings_paint->hide();
}

void MapView::open_help()
{
  mainGui->showHelp();
}

//! \todo Make this a member of MapView. Also correctly add the actions below again.
/*!
  \brief Import a new model form a text file or a hard coded one.
  Imports a model from the import.txt, the wowModelViewer log or just insert some hard coded testing models.
  \param id the id switch the import kind
*/
/*void InsertObject( UIFrame* button, int id )
{
  //! \todo Beautify.

  // Test if there is an selection
  if( !_world->HasSelection() )
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
       case 15:
        if( boost::filesystem::exists( "noggIt.conf" ) )
        {
          ConfigFile config( "noggIt.conf" );
          config.readInto( importFile, "ImportFile" );
        }
       break;

       case 1:
        importFile="Import.txt"; //  use import.txt in noggit folder!
       break;

       default:
        m2s_to_add.push_back( filesToAdd[id] );
        break;
  }

  std::string lastModel;
  std::string lastWMO;

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
          findThis =   "Loading model: ";
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
          lastWMO = line;
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
  switch( _world->GetCurrentSelection()->type )
  {
    case eEntry_Model:
      selectionPosition = _world->GetCurrentSelection()->data.model->pos;
      break;
    case eEntry_WMO:
      selectionPosition = _world->GetCurrentSelection()->data.wmo->pos;
      break;
    case eEntry_MapChunk:
      selectionPosition = _world->GetCurrentSelection()->data.mapchunk->GetSelectionPosition();
      break;
  }


  if(id==14)
  {
    LogError << "M2 Problem 14:" << lastModel << " - " << id << std::endl;
    if(lastModel!="")
      if( !MPQFile::exists(lastModel) )
        LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
      else
        _world->addM2( ModelManager::add( lastModel ), selectionPosition );
  }
  else if(id==15)
  {
      LogError << "M2 Problem 15:" << lastModel << " - " << id << std::endl;
    if(lastWMO!="")
      if( !MPQFile::exists(lastWMO) )
        LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
      else
        _world->addWMO( WMOManager::add( lastWMO ), selectionPosition );
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

      _world->addWMO( WMOManager::add( *it ), selectionPosition );
    }

    for( std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it )
    {

      if( !MPQFile::exists(*it) )
      {

        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      _world->addM2( ModelManager::add( *it ), selectionPosition );
    }
  }
  //! \todo Memoryleak: These models will never get deleted.
}
*/

void view_texture_palette( UIFrame* /*button*/, int /*id*/ )
{
  mainGui->TexturePalette->toggleVisibility();
}

void MapView::TEST_save_wdt()
{
  _world->saveWDT();
}

void MapView::move_heightmap()
{
  // set areaid on all chunks of the current ADT
  if(Environment::getInstance()->selectedAreaID)
    _world->moveHeight(Environment::getInstance()->selectedAreaID ,misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::clear_heightmap()
{
  // set areaid on all chunks of the current ADT
  if(Environment::getInstance()->selectedAreaID)
    _world->clearHeight(Environment::getInstance()->selectedAreaID ,misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::set_area_id()
{
  // set areaid on all chunks of the current ADT
  if(Environment::getInstance()->selectedAreaID)
    _world->setAreaID(Environment::getInstance()->selectedAreaID ,misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::clear_all_models()
{
  // call the clearAllModelsOnADT method to clear them all on current ADT
    _world->clearAllModelsOnADT(misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void changeZoneIDValue(UIFrame* /*f*/,int set)
{
  Environment::getInstance()->selectedAreaID = set;
  if( Environment::getInstance()->areaIDColors.find(set) == Environment::getInstance()->areaIDColors.end() )
  {
    Vec3D newColor = Vec3D( misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) );
    Environment::getInstance()->areaIDColors.insert( std::pair<int,Vec3D>(set, newColor) );
  }
}

void MapView::clear_texture()
{
  // set areaid on all chunks of the current ADT
  _world->setBaseTexture(misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::show_texture_switcher()
{
  mainGui->TextureSwitcher->getTextures(_world->GetCurrentSelection());
  //mainGui->TextureSwitcher->show();
}

void MapView::show_cursor_switcher()
{
  mainGui->showCursorSwitcher();
}

#ifdef __FILESAREMISSING

//! \todo make this a member of MapView.
std::string getCurrentHeightmapPath()
{
  // get MapName
  std::string mapName;
  int id = _world->getMapID();
  for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i )
  {
    if( i->getInt( MapDB::MapID ) == id)
      mapName = i->getString( MapDB::InternalName );
  }

  // build the path and filename string.
  std::stringstream png_filename;
  png_filename << Project::getInstance()->getPath() << "world\\maps\\" << mapName << "\\H_" << mapName
    << "_" << misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE) << "_" << misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE) << ".png" ;
  return png_filename.str();

}

void MapView::export_heightmap()
{

  // create the image and write to disc.
  GLfloat* data = new GLfloat[272*272];

  ilInit();

  int width  = 272 ;
  int height = 272 ;
  int bytesToUsePerPixel = 32;  // 16 bit per channel
  int sizeOfByte = sizeof( unsigned char ) ;
  int theSize = width * height * sizeOfByte * bytesToUsePerPixel ;

  unsigned char * imData =(unsigned char*)malloc( theSize ) ;

  int colors = 0;
  // write the height data to the image array
  for( int i = 0 ; i < theSize ; i++ )
  {
    imData[ i ] = colors ;
    if(i==100)colors = 200;
    if(i==200)colors = 4000;
  }


  ILuint ImageName; // The image name.
  ilGenImages(1, &ImageName); // Grab a new image name.
  ilBindImage(ImageName); // bind it
  ilTexImage(width,height,1,bytesToUsePerPixel,GL_LUMINANCE,IL_UNSIGNED_BYTE,NULL);
  ilSetData(imData);
  ilEnable(IL_FILE_OVERWRITE);
  //ilSave(IL_PNG, getCurrentHeightmapPath().c_str());
  ilSave(IL_PNG, "test2.png");
  free(imData);
}

void MapView::import_heightmap()
{
  ilInit();

  //ILboolean loadImage = ilLoadImage( getCurrentHeightmapPath().c_str() ) ;
  const char *image = "test.png";
  ILboolean loadImage = ilLoadImage( image ) ;

  std::stringstream MessageText;
  if(loadImage)
  {

    LogDebug << "Image loaded: " << image << "\n";
    LogDebug <<  "ImageSize: " << ilGetInteger( IL_IMAGE_SIZE_OF_DATA ) << "\n";
    LogDebug <<  "BPP: " << ilGetInteger( IL_IMAGE_BITS_PER_PIXEL ) << "\n";
    LogDebug <<  "Format: " << ilGetInteger( IL_IMAGE_FORMAT ) << "\n";
    LogDebug <<  "SizeofData: " << ilGetInteger( IL_IMAGE_SIZE_OF_DATA ) << "\n";

  }
  else
  {
    LogDebug << "Cant load Image: " << image << "\n";
    ILenum err = ilGetError() ;

    MessageText << err << "\n";
    //MessageText << ilGetString(ilGetError()) << "\n";
    LogDebug << MessageText.str();
  }
}
#else
void MapView::import_heightmap() { }
void MapView::export_heightmap() { }
#endif

void MapView::createGUI()
{
  // create main gui object that holds all other gui elements for access ( in the future ;) )
  mainGui = new UIMapViewGUI (_world, this);
  mainGui->guiToolbar->current_texture->setClickFunc( view_texture_palette, 0 );

  mainGui->ZoneIDBrowser->setMapID( _world->getMapID() );
  mainGui->ZoneIDBrowser->setChangeFunc( changeZoneIDValue );

  _doodad_spawner = new UIDoodadSpawner (_world);
  _doodad_spawner->hide();
  mainGui->addChild (_doodad_spawner);

  tool_settings_x = video.xres() - 186;
  tool_settings_y = 38;

  // Raise/Lower
  setting_ground=new UIWindow( tool_settings_x, tool_settings_y, 180.0f, 160.0f );
  setting_ground->movable( true );
  mainGui->addChild( setting_ground );

  setting_ground->addChild( new UIText( 78.5f, 2.0f, "Raise / Lower", arial14, eJustifyCenter ) );

  gGroundToggleGroup = new UIToggleGroup( &groundBrushType );
  setting_ground->addChild( new UICheckBox( 6.0f, 15.0f, "Flat", gGroundToggleGroup, 0 ) );
  setting_ground->addChild( new UICheckBox( 85.0f, 15.0f, "Linear", gGroundToggleGroup, 1 ) );
  setting_ground->addChild( new UICheckBox( 6.0f, 40.0f, "Smooth", gGroundToggleGroup, 2 ) );
  setting_ground->addChild( new UICheckBox( 85.0f, 40.0f, "Polynomial", gGroundToggleGroup, 3 ) );
  setting_ground->addChild( new UICheckBox( 6.0f, 65.0f, "Trigonom", gGroundToggleGroup, 4 ) );
  setting_ground->addChild( new UICheckBox( 85.0f, 65.0f, "Quadratic", gGroundToggleGroup, 5 ) );
  gGroundToggleGroup->Activate(1);

  ground_brush_radius=new UISlider(6.0f,120.0f,167.0f,1000.0f,0.00001f);
  ground_brush_radius->setFunc(setGroundBrushRadius);
  ground_brush_radius->setValue(groundBrushRadius/1000);
  ground_brush_radius->setText( "Brush radius: " );
  setting_ground->addChild(ground_brush_radius);

  ground_brush_speed=new UISlider(6.0f,145.0f,167.0f,10.0f,0.00001f);
  ground_brush_speed->setFunc(setGroundBrushSpeed);
  ground_brush_speed->setValue(groundBrushSpeed/10);
  ground_brush_speed->setText( "Brush Speed: " );
  setting_ground->addChild(ground_brush_speed);

  // flatten/blur
  setting_blur=new UIWindow(tool_settings_x,tool_settings_y,180.0f,100.0f);
  setting_blur->movable( true );
  setting_blur->hide();
  mainGui->addChild(setting_blur);

  setting_blur->addChild( new UIText( 78.5f, 2.0f, "Flatten / Blur", arial14, eJustifyCenter ) );

  gBlurToggleGroup = new UIToggleGroup( &blurBrushType );
  setting_blur->addChild( new UICheckBox( 6.0f, 15.0f, "Flat", gBlurToggleGroup, 0 ) );
  setting_blur->addChild( new UICheckBox( 80.0f, 15.0f, "Linear", gBlurToggleGroup, 1 ) );
  setting_blur->addChild( new UICheckBox( 6.0f, 40.0f, "Smooth", gBlurToggleGroup, 2 ) );
  gBlurToggleGroup->Activate( 1 );

  blur_brush=new UISlider(6.0f,85.0f,167.0f,1000.0f,0.00001f);
  blur_brush->setFunc(setBlurBrushRadius);
  blur_brush->setValue(blurBrushRadius/1000);
  blur_brush->setText( "Brush radius: " );
  setting_blur->addChild(blur_brush);

  //3D Paint settings UIWindow
  settings_paint=new UIWindow(tool_settings_x,tool_settings_y,180.0f,140.0f);
  settings_paint->hide();
  settings_paint->movable( true );

  mainGui->addChild(settings_paint);

  settings_paint->addChild( new UIText( 78.5f, 2.0f, "3D Paint", arial14, eJustifyCenter ) );

  UIGradient *G1;
  G1=new UIGradient;
  G1->width( 20.0f );
  G1->x( settings_paint->width() - 4.0f - G1->width() );
  G1->y( 4.0f );
  G1->height( 92.0f );
  G1->setMaxColor(1.0f,1.0f,1.0f,1.0f);
  G1->setMinColor(0.0f,0.0f,0.0f,1.0f);
  G1->horiz=false;
  G1->setClickColor(1.0f,0.0f,0.0f,1.0f);
  G1->setClickFunc(setTextureBrushLevel);
  G1->setValue(0.0f);

  settings_paint->addChild(G1);

  UISlider* S1;
  S1=new UISlider(6.0f,33.0f,145.0f,1.0f,0.0f);
  S1->setFunc(setTextureBrushHardness);
  S1->setValue(textureBrush.getHardness());
  S1->setText("Hardness: ");
  settings_paint->addChild(S1);

  paint_brush=new UISlider(6.0f,59.0f,145.0f,100.0f,0.00001);
  paint_brush->setFunc(setTextureBrushRadius);
  paint_brush->setValue(textureBrush.getRadius() / 100 );
  paint_brush->setText("Radius: ");
  settings_paint->addChild(paint_brush);

  S1=new UISlider(6.0f,85.0f,145.0f,0.99f,0.01f);
  S1->setFunc(setTextureBrushPressure);
  S1->setValue(brushPressure);
  S1->setText("Pressure: ");
  settings_paint->addChild(S1);


  UIButton* B1;
  B1=new UIButton( 6.0f, 111.0f, 170.0f, 30.0f, "Texture swapper", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", openSwapper, 1 ) ;
  settings_paint->addChild(B1);

  mainGui->addChild(mainGui->TexturePalette = UITexturingGUI::createTexturePalette(4,8,mainGui));
  mainGui->TexturePalette->hide();
  mainGui->addChild(mainGui->SelectedTexture = UITexturingGUI::createSelectedTexture());
  mainGui->SelectedTexture->hide();
  mainGui->addChild(UITexturingGUI::createTilesetLoader());
  mainGui->addChild(UITexturingGUI::createTextureFilter());
  mainGui->addChild(MapChunkWindow = UITexturingGUI::createMapChunkWindow());
  MapChunkWindow->hide();

#define NEW_ACTION(__NAME__, __TEXT, __SLOT, __KEYS) QAction* __NAME__ (new_action (__TEXT, __SLOT, __KEYS));
#define NEW_TOGGLE_ACTION(__NAME__, __TEXT, __SLOT, __KEYS, __DEFAULT) QAction* __NAME__ (new_toggleable_action (__TEXT, __SLOT, __DEFAULT, __KEYS));

  NEW_ACTION (save_current_tile, tr ("Save current tile"), SLOT (save()), Qt::CTRL + Qt::SHIFT + Qt::Key_S);
  NEW_ACTION (save_modified_tiles, tr ("Save all modified tiles"), SLOT (save_all()), QKeySequence::Save);
  NEW_ACTION (reload_tile, tr ("Reload current tile"), SLOT (reload_current_tile()), Qt::SHIFT + Qt::Key_J);

  NEW_ACTION (bookmark, tr ("Add bookmark for this location"), SLOT (add_bookmark()), Qt::Key_F5);

  NEW_ACTION (export_heightmap, tr ("Export heightmap"), SLOT (export_heightmap()), 0);
  NEW_ACTION (import_heightmap, tr ("Import heightmap"), SLOT (import_heightmap()), 0);

  NEW_ACTION (to_menu, tr ("Return to menu"), SLOT (exit_to_menu()), Qt::Key_Escape);


  NEW_ACTION (copy_object, tr ("Copy object"), SLOT (copy_selected_object()), QKeySequence::Copy);
  NEW_ACTION (paste_object, tr ("Paste object"), SLOT (paste_object()), QKeySequence::Paste);
  NEW_ACTION (delete_object, tr ("Delete object"), SLOT (delete_selected_object()), QKeySequence::Delete);
  //delete_object->setShortcuts (QList<QKeySequence>() << QKeySequence::Delete << Qt::Key_Backspace);

  NEW_ACTION (reset_rotation, tr ("Reset object's rotation"), SLOT (reset_selected_object_rotation()), Qt::CTRL + Qt::Key_R);
  NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);


  NEW_TOGGLE_ACTION (toolbar, tr ("Toolbar"), SLOT (toggle_toolbar_visibility (bool)), 0, true);
  NEW_TOGGLE_ACTION (current_texture, tr ("Selected texture"), SLOT (toggle_current_texture_visiblity (bool)), 0, false);
  NEW_TOGGLE_ACTION (map_chunk_settings, tr ("Map chunk settings"), SLOT (show_map_chunk_settings(bool)), 0, false);
  NEW_TOGGLE_ACTION (toggle_minimap, tr ("Show minimap"), SLOT (toggle_minimap (bool)), Qt::Key_M, false);
  NEW_TOGGLE_ACTION (detail_infos, tr ("Object information"), SLOT (toggle_detail_info_window (bool)), Qt::Key_F8, false);


  NEW_TOGGLE_ACTION (doodad_drawing, tr ("Draw doodads"), SLOT (toggle_doodad_drawing (bool)), Qt::Key_F1, true);
  NEW_TOGGLE_ACTION (wmo_doodad_drawing, tr ("Draw doodads inside of WMOs"), SLOT (toggle_wmo_doodad_drawing (bool)), Qt::Key_F2, true);
  NEW_TOGGLE_ACTION (terrain_drawing, tr ("Draw terrain"), SLOT (toggle_terrain_drawing (bool)), Qt::Key_F3, true);
  NEW_TOGGLE_ACTION (water_drawing, tr ("Draw water"), SLOT (toggle_water_drawing (bool)), Qt::Key_F4, true);
  NEW_TOGGLE_ACTION (wmo_drawing, tr ("Draw WMOs"), SLOT (toggle_wmo_drawing (bool)), Qt::Key_F6, true);
  NEW_TOGGLE_ACTION (line_drawing, tr ("Draw lines"), SLOT (toggle_line_drawing (bool)), Qt::Key_F7, false);
  NEW_TOGGLE_ACTION (hole_line_drawing, tr ("Draw lines for holes"), SLOT (toggle_hole_line_drawing (bool)), Qt::SHIFT + Qt::Key_F7, false);
  //! \todo on OSX this shows up as "8" in menu and does not react to the keybinding.
  NEW_TOGGLE_ACTION (contour_drawing, tr ("Draw contours"), SLOT (toggle_contour_drawing (bool)), Qt::Key_F9, false);
  NEW_TOGGLE_ACTION (fog_drawing, tr ("Draw fog"), SLOT (toggle_fog_drawing (bool)), Qt::Key_F, true);
  NEW_TOGGLE_ACTION (toggle_lighting, tr ("Enable Lighting"), SLOT (toggle_lighting (bool)), Qt::Key_L, true);

  NEW_ACTION (turn_around, tr ("Turn camera 180 degrees"), SLOT (turn_around()), Qt::Key_R);


  NEW_ACTION (cursor_selector, tr ("Choose selection cursor"), SLOT (show_cursor_switcher()), 0);
  NEW_TOGGLE_ACTION (invert_mouse_y_axis, tr ("Invert mouse y-axis"), SLOT (invert_mouse_y_axis (bool)), Qt::Key_I, false);
  NEW_TOGGLE_ACTION (auto_selection, tr ("Automatic selection"), SLOT (toggle_auto_selecting (bool)), Qt::SHIFT + Qt::Key_F4, false);

  NEW_TOGGLE_ACTION (rotation_randomization, tr ("Randomized rotation when copying"), SLOT (toggle_copy_rotation_randomization (bool)), 0, false);
  NEW_TOGGLE_ACTION (position_randomization, tr ("Randomized position when copying"), SLOT (toggle_copy_position_randomization (bool)), 0, false);
  NEW_TOGGLE_ACTION (size_randomization, tr ("Randomized size when copying"), SLOT (toggle_copy_size_randomization (bool)), 0, false);

  NEW_ACTION (decrease_time_speed, tr ("Decrease time speed"), SLOT (decrease_time_speed()), Qt::Key_B);
  NEW_ACTION (increase_time_speed, tr ("Increase time speed"), SLOT (increase_time_speed()), Qt::Key_N);
  NEW_ACTION (decrease_moving_speed, tr ("Decrease movement speed"), SLOT (decrease_moving_speed()), Qt::Key_O);
  NEW_ACTION (increase_moving_speed, tr ("Increase movement speed"), SLOT (increase_moving_speed()), Qt::Key_P);


  NEW_ACTION (key_bindings, tr ("Key bindings"), SLOT (open_help()), Qt::Key_H);
  NEW_TOGGLE_ACTION (application_infos, tr ("Show application infos"), SLOT (toggle_app_info (bool)), 0, false);


  NEW_ACTION (save_wdt, tr ("Save WDT"), SLOT (TEST_save_wdt()), 0);
  NEW_ACTION (save_minimap, tr ("Save minimap as raw files"), SLOT (save_minimap()), Qt::Key_P + Qt::SHIFT + Qt::CTRL);
  NEW_ACTION (toggle_doodad_spawner, tr ("toggle_doodad_spawner"), SLOT (toggle_doodad_spawner()), Qt::Key_T);

#undef NEW_ACTION
#undef NEW_TOGGLE_ACTION

  QMenuBar* menu_bar (new QMenuBar (NULL));

  QMenu* file_menu (menu_bar->addMenu (tr ("File")));
  file_menu->addAction (save_current_tile);
  file_menu->addAction (save_modified_tiles);
  file_menu->addAction (reload_tile);
  file_menu->addSeparator();
  file_menu->addAction (bookmark);
  file_menu->addSeparator();
  file_menu->addAction (export_heightmap);
  file_menu->addAction (import_heightmap);
  file_menu->addSeparator();
  file_menu->addAction (to_menu);

  QMenu* edit_menu (menu_bar->addMenu (tr ("Edit")));
  edit_menu->addAction (copy_object);
  edit_menu->addAction (paste_object);
  edit_menu->addAction (delete_object);
  edit_menu->addSeparator();
  edit_menu->addAction (reset_rotation);
  edit_menu->addAction (snap_object_to_ground);

  QMenu* assist_menu (menu_bar->addMenu (tr ("Assist")));
  QMenu* insertion_menu (assist_menu->addMenu (tr ("Insert helper model")));

  /*
  assist_menu->addAction (tr ("all from MV", InsertObject, 0  );
  assist_menu->addAction (tr ("last M2 from MV", InsertObject, 14  );
  assist_menu->addAction (tr ("last WMO from MV", InsertObject, 15  );
  assist_menu->addAction (tr ("from Text File", InsertObject, 1  );
  insertion_menu->addAction (tr ("Human scale", InsertObject, 2  );
  insertion_menu->addAction (tr ("Cube 50", InsertObject, 3  );
  insertion_menu->addAction (tr ("Cube 100", InsertObject, 4  );
  insertion_menu->addAction (tr ("Cube 250", InsertObject, 5  );
  insertion_menu->addAction (tr ("Cube 500", InsertObject, 6  );
  insertion_menu->addAction (tr ("Cube 1000", InsertObject, 7  );
  insertion_menu->addAction (tr ("Disc 50", InsertObject, 8  );
  insertion_menu->addAction (tr ("Disc 200", InsertObject, 9  );
  insertion_menu->addAction (tr ("Disc 777", InsertObject, 10  );
  insertion_menu->addAction (tr ("Sphere 50", InsertObject, 11  );
  insertion_menu->addAction (tr ("Sphere 200", InsertObject, 12  );
  insertion_menu->addAction (tr ("Sphere 777", InsertObject, 13  );
  assist_menu->addSeparator();
  assist_menu->addAction (tr ("Set Area ID", set_area_id, 0  );
  assist_menu->addAction (tr ("Clear height map", clear_heightmap, 0  );
  assist_menu->addAction (tr ("Move to position", move_heightmap, 0  );
  assist_menu->addAction (tr ("Clear texture", clear_texture, 0  );
  assist_menu->addAction (tr ("Clear models", clear_all_models, 0  );
  assist_menu->addAction (tr ("Switch texture", show_texture_switcher, 0  );
*/
  QMenu* view_menu (menu_bar->addMenu (tr ("View")));
  view_menu->addAction (toolbar);
  view_menu->addAction (current_texture);
  view_menu->addAction (toggle_minimap);
  view_menu->addAction (detail_infos);
  //! \todo re-add this when reimplemented.
  //view_menu->addAction (map_chunk_settings);
  view_menu->addSeparator();
  view_menu->addAction (doodad_drawing);
  view_menu->addAction (wmo_doodad_drawing);
  view_menu->addAction (terrain_drawing);
  view_menu->addAction (water_drawing);
  view_menu->addAction (wmo_drawing);
  view_menu->addAction (line_drawing);
  view_menu->addAction (hole_line_drawing);
  view_menu->addAction (contour_drawing);
  view_menu->addAction (fog_drawing);
  view_menu->addAction (toggle_lighting);

  QMenu* settings_menu (menu_bar->addMenu (tr ("Settings")));
  settings_menu->addAction (cursor_selector);
  settings_menu->addSeparator();
  settings_menu->addAction (rotation_randomization);
  settings_menu->addAction (position_randomization);
  settings_menu->addAction (size_randomization);
  settings_menu->addSeparator();
  settings_menu->addAction (auto_selection);
  settings_menu->addAction (invert_mouse_y_axis);
  settings_menu->addSeparator();
  settings_menu->addAction (decrease_time_speed);
  settings_menu->addAction (increase_time_speed);
  settings_menu->addAction (decrease_moving_speed);
  settings_menu->addAction (increase_moving_speed);

  QMenu* help_menu (menu_bar->addMenu (tr ("Help")));
  help_menu->addAction (key_bindings);
  help_menu->addAction (application_infos);

  QMenu* debug_menu (menu_bar->addMenu (tr ("Testing and Debugging")));
  debug_menu->addAction (save_wdt);
  debug_menu->addAction (toggle_doodad_spawner);
  debug_menu->addAction (save_minimap);

  QMenu* useless_menu (debug_menu->addMenu (tr ("Stuff that should only be on keys")));
  useless_menu->addAction (turn_around);

  // Minimap window
  _minimap = new minimap_widget (NULL);
  _minimap->world (_world);
  _minimap->draw_skies (true);
  _minimap->draw_camera (true);
  _minimap->draw_boundaries (true);
  _minimap->hide();
}

QAction* MapView::new_action (const QString& text, const char* slot, const QKeySequence& shortcut)
{
  QAction* action (new QAction (text, this));
  connect (action, SIGNAL (triggered()), slot);
  if (shortcut != QKeySequence (0))
  {
    action->setShortcuts (QList<QKeySequence>() << shortcut);
  }
  return action;
}

QAction* MapView::new_toggleable_action (const QString& text, const char* slot, bool default_value, const QKeySequence& shortcut)
{
  QAction* action (new QAction (text, this));
  connect (action, SIGNAL (toggled (bool)), slot);
  action->setCheckable (true);
  action->setChecked (default_value);
  if (shortcut != QKeySequence (0))
  {
    action->setShortcuts (QList<QKeySequence>() << shortcut);
  }
  return action;
}

void MapView::show_map_chunk_settings (bool value)
{
  MapChunkWindow->hidden (!value);
}

void MapView::toggle_toolbar_visibility (bool value)
{
  mainGui->guiToolbar->hidden (!value);
}

void MapView::toggle_current_texture_visiblity (bool value)
{
  mainGui->SelectedTexture->hidden (!value);
}

void MapView::toggle_copy_size_randomization (bool value)
{
  Settings::getInstance()->copy_size = value;
}
void MapView::toggle_copy_position_randomization (bool value)
{
  Settings::getInstance()->copy_tile = value;
}
void MapView::toggle_copy_rotation_randomization (bool value)
{
  Settings::getInstance()->copy_rot = value;
}

void MapView::toggle_app_info (bool visiblity)
{
  mainGui->guiappInfo->hidden (!visiblity);
}

MapView::MapView (World* world, float ah0, float av0, QGLWidget* shared, QWidget* parent)
  : QGLWidget (parent, shared)
  , _startup_time ()
  , _last_update (0.0)
  , ah( ah0 )
  , av( av0 )
  , _world (world)
  , _GUIDisplayingEnabled( true )
  , mTimespeed( 0.0f )
{
  LastClicked=0;

  moving = strafing = updown = 0.0f;

  mousedir = -1.0f;

  movespd = SPEED;

  lastBrushUpdate = 0;
  textureBrush.init();

  look = false;
  mViewMode = eViewMode_3D;

  createGUI();

  startTimer (40);
  _startup_time.start();

  setFocusPolicy (Qt::StrongFocus);
  setMouseTracking (true);
}

void MapView::timerEvent (QTimerEvent*)
{
  const qreal now (_startup_time.elapsed() / 1000.0);

  tick (now, now - _last_update);
  updateGL();

  _last_update = now;
}

void MapView::initializeGL()
{
  qglClearColor (Qt::black);

//! \todo remove these?
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_MULTISAMPLE);
  static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
  glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
}

void MapView::paintGL()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  display();
}

void MapView::resizeGL (int width, int height)
{
  video.init (width, height);
}

MapView::~MapView()
{
  delete mainGui;
  mainGui = NULL;

  delete _world;
  _world = NULL;
}

void MapView::tick( float t, float dt )
{
  dt = std::min( dt, 1.0f );

  Vec3D dir( 1.0f, 0.0f, 0.0f );
  Vec3D dirUp( 1.0f, 0.0f, 0.0f );
  Vec3D dirRight( 0.0f, 0.0f, 1.0f );
  rotate( 0.0f, 0.0f, &dir.x,&dir.y, av * PI / 180.0f );
  rotate( 0.0f, 0.0f, &dir.x,&dir.z, ah * PI / 180.0f );

  if( Environment::getInstance()->ShiftDown )
  {
    dirUp.x = 0.0f;
    dirUp.y = 1.0f;
    dirRight *= 0.0f; //! \todo  WAT?
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

  nameEntry * Selection = _world->GetCurrentSelection();

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
        _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
        Selection->data.wmo->pos.x += keyx * moveratio;
        Selection->data.wmo->pos.y += keyy * moveratio;
        Selection->data.wmo->pos.z += keyz * moveratio;
        Selection->data.wmo->dir.y += keyr * moveratio * 2;
        _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
      }

      if( Selection->type == eEntry_Model )
      {
        _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
        Selection->data.model->pos.x += keyx * moveratio;
        Selection->data.model->pos.y += keyy * moveratio;
        Selection->data.model->pos.z += keyz * moveratio;
        Selection->data.model->dir.y += keyr * moveratio * 2;
        Selection->data.model->sc += keys * moveratio / 50;
        _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
      }
    }

    Vec3D ObjPos;
    if( _world->IsSelection( eEntry_Model ) )
    {
      //! \todo  Tell me what this is.
      ObjPos = Selection->data.model->pos - _world->camera;
      rotate( 0.0f, 0.0f, &ObjPos.x, &ObjPos.y, av * PI / 180.0f );
      rotate( 0.0f, 0.0f, &ObjPos.x, &ObjPos.z, ah * PI / 180.0f );
      ObjPos.x = abs( ObjPos.x );
    }

    // moving and scaling objects
    //! \todo  Alternatively automatically align it to the terrain. Also try to move it where the mouse points.
    if( MoveObj )
      if( Selection->type == eEntry_WMO )
      {
         _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z); // before move
         ObjPos.x = 80.0f;
         Selection->data.wmo->pos+=mv * dirUp * ObjPos.x;
         Selection->data.wmo->pos-=mh * dirRight * ObjPos.x;
         Selection->data.wmo->extents[0] = Selection->data.wmo->pos - Vec3D(1,1,1);
         Selection->data.wmo->extents[1] = Selection->data.wmo->pos + Vec3D(1,1,1);
         _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z); // after move. If moved to another ADT
      }
      else if( Selection->type == eEntry_Model )
        if( Environment::getInstance()->AltDown )
        {
          _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
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
          _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z); // before move
          ObjPos.x = 80.0f;
          Selection->data.model->pos += mv * dirUp * ObjPos.x;
          Selection->data.model->pos -= mh * dirRight * ObjPos.x;
          _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z); // after move. If moved to another ADT
        }


    // rotating objects
    if( look )
    {
      float * lTarget = NULL;
      bool lModify = false;

      if( Selection->type == eEntry_Model )
      {
        _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
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
        _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
        lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
        if( Environment::getInstance()->ShiftDown )
          lTarget = &Selection->data.wmo->dir.y;
        else if( Environment::getInstance()->CtrlDown )
          lTarget = &Selection->data.wmo->dir.x;
        else if( Environment::getInstance()->AltDown )
          lTarget = &Selection->data.wmo->dir.z;
      }

      if( lModify && lTarget )
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



      xPos = Environment::getInstance()->Pos3DX;
      yPos = Environment::getInstance()->Pos3DY;
      zPos = Environment::getInstance()->Pos3DZ;

      switch( terrainMode )
      {
      case 0:
        if( Environment::getInstance()->ShiftDown )
        {
          // Move ground up
          if( mViewMode == eViewMode_3D ) _world->changeTerrain( xPos, zPos, 7.5f * dt * groundBrushSpeed, groundBrushRadius, groundBrushType );
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          // Move ground down
          if( mViewMode == eViewMode_3D ) _world->changeTerrain( xPos, zPos, -7.5f * dt * groundBrushSpeed, groundBrushRadius, groundBrushType );
        }
      break;

      case 1:
        if( Environment::getInstance()->ShiftDown )
          if( mViewMode == eViewMode_3D ) _world->flattenTerrain( xPos, zPos, yPos, pow( 0.2f, dt ), blurBrushRadius, blurBrushType );
        if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D ) _world->blurTerrain( xPos, zPos, pow( 0.2f, dt ), std::min( blurBrushRadius, 30.0f ), blurBrushType );
        }
      break;

      case 2:
        if( Environment::getInstance()->ShiftDown && Environment::getInstance()->CtrlDown)
        {
          // clear chunk texture
          if( mViewMode == eViewMode_3D )
            _world->eraseTextures(xPos, zPos);
          else if( mViewMode == eViewMode_2D )
            _world->eraseTextures( CHUNKSIZE * 4.0f * video.ratio() * ( static_cast<float>( MouseX ) / static_cast<float>( video.xres() ) - 0.5f ) / _world->zoom+_world->camera.x, CHUNKSIZE * 4.0f * ( static_cast<float>( MouseY ) / static_cast<float>( video.yres() ) - 0.5f) / _world->zoom+_world->camera.z );
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          // Pick texture
          mainGui->TexturePicker->getTextures( _world->GetCurrentSelection());
        }
        else  if( Environment::getInstance()->ShiftDown)
        {
          // Paint 3d if shift down.
          if( UITexturingGUI::getSelectedTexture() )
          {
            if( textureBrush.needUpdate() )
            {
              textureBrush.GenerateTexture();
            }
            if( mViewMode == eViewMode_3D )
              _world->paintTexture( xPos, zPos, &textureBrush, brushLevel, 1.0f - pow( 1.0f - brushPressure, dt * 10.0f ), UITexturingGUI::getSelectedTexture() );
          }
        }
        else
        {
          // paint 2d if nothing is pressed.
          if( textureBrush.needUpdate() )
          {
            textureBrush.GenerateTexture();
          }
          if( mViewMode == eViewMode_2D && !(Environment::getInstance()->ShiftDown) )
            _world->paintTexture( CHUNKSIZE * 4.0f * video.ratio() * ( static_cast<float>( MouseX ) / static_cast<float>( video.xres() ) - 0.5f ) / _world->zoom+_world->camera.x, CHUNKSIZE * 4.0f * ( static_cast<float>( MouseY ) / static_cast<float>( video.yres() ) - 0.5f) / _world->zoom+_world->camera.z , &textureBrush, brushLevel, 1.0f - pow( 1.0f - brushPressure, dt * 10.0f ), UITexturingGUI::getSelectedTexture() );
        }
      break;

      case 3:
        if( Environment::getInstance()->ShiftDown  )
        {
      // if there is no terain the projection mothod dont work. So get the cords by selection.
    Selection->data.mapchunk->getSelectionCoord( &xPos, &zPos );
    yPos = Selection->data.mapchunk->getSelectionHeight();
          if( mViewMode == eViewMode_3D )      _world->removeHole( xPos, zPos );
          //else if( mViewMode == eViewMode_2D )  _world->removeHole( CHUNKSIZE * 4.0f * video.ratio() * ( float( MouseX ) / float( video.xres() ) - 0.5f ) / _world->zoom+_world->camera.x, CHUNKSIZE * 4.0f * ( float( MouseY ) / float( video.yres() ) - 0.5f) / _world->zoom+_world->camera.z );
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D )      _world->addHole( xPos, zPos );
          //else if( mViewMode == eViewMode_2D )  _world->addHole( CHUNKSIZE * 4.0f * video.ratio() * ( float( MouseX ) / float( video.xres() ) - 0.5f ) / _world->zoom+_world->camera.x, CHUNKSIZE * 4.0f * ( float( MouseY ) / float( video.yres() ) - 0.5f) / _world->zoom+_world->camera.z );
        }
      break;

      case 4:
        if( Environment::getInstance()->ShiftDown  )
        {
          if( mViewMode == eViewMode_3D )
          {
          // draw the selected AreaId on current selected chunk
          nameEntry * lSelection = _world->GetCurrentSelection();
          int mtx,mtz,mcx,mcy;
          mtx = lSelection->data.mapchunk->mt->mPositionX;
          mtz = lSelection->data.mapchunk->mt->mPositionZ ;
          mcx = lSelection->data.mapchunk->px;
          mcy = lSelection->data.mapchunk->py;
          _world->setAreaID( Environment::getInstance()->selectedAreaID, mtx,mtz, mcx, mcy );
          }
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D )
          {
          // pick areaID from chunk
          int newID = _world->GetCurrentSelection()->data.mapchunk->areaID;
          Environment::getInstance()->selectedAreaID = newID;
          mainGui->ZoneIDBrowser->setZoneID(newID);
          }
        }

      break;

      case 5:
        if( Environment::getInstance()->ShiftDown  )
        {
          if( mViewMode == eViewMode_3D ) _world->setFlag( true, xPos, zPos );
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D ) _world->setFlag( false, xPos, zPos );
        }
      break;
      }
    }
  }

  if( mViewMode != eViewMode_2D )
  {
    if( moving )
      _world->camera += dir * dt * movespd * moving;
    if( strafing )
    {
      Vec3D right = dir % Vec3D( 0.0f, 1.0f ,0.0f );
      right.normalize();
      _world->camera += right * dt * movespd * strafing;
    }
    if( updown )
      _world->camera += Vec3D( 0.0f, dt * movespd * updown, 0.0f );

    _world->lookat = _world->camera + dir;
  }
  else
  {
    if( moving )
      _world->camera.z -= dt * movespd * moving / ( _world->zoom * 1.5f );
    if( strafing )
      _world->camera.x += dt * movespd * strafing / ( _world->zoom * 1.5f );
    if( updown )
      _world->zoom *= pow( 2.0f, dt * updown * 4.0f );

    _world->zoom = std::min( std::max( _world->zoom, 0.1f ), 2.0f );
  }


  if( ( t - lastBrushUpdate ) > 0.1f && textureBrush.needUpdate() )
  {
    textureBrush.GenerateTexture();
  }

  _world->time += mTimespeed * dt;
  _world->animtime += dt * 1000.0f;
  globalTime = static_cast<int>( _world->animtime );

  _world->tick(dt);

  if( !MapChunkWindow->hidden() && _world->GetCurrentSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk )
  {
    UITexturingGUI::setChunkWindow( _world->GetCurrentSelection()->data.mapchunk );
  }
}

void MapView::doSelection( bool selectTerrainOnly )
{
  _world->drawSelection( MouseX, MouseY, selectTerrainOnly );
}


void MapView::displayGUIIfEnabled()
{
  if( _GUIDisplayingEnabled )
  {
    video.set2D();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    OpenGL::Texture::disableTexture( 1 );
    OpenGL::Texture::enableTexture( 0 );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

    OpenGL::Texture::disableTexture( 0 );

    mainGui->setTilemode( mViewMode != eViewMode_3D );
    mainGui->render();

    OpenGL::Texture::enableTexture( 0 );
  }

  //! \todo This should only be done when actually needed. (on movement and camera changes as well as modifying an adt)
  _minimap->update();
}

void MapView::displayViewMode_2D()
{
  video.setTileMode();
  _world->drawTileMode( ah );

  const float mX = ( CHUNKSIZE * 4.0f * video.ratio() * ( static_cast<float>( MouseX ) / static_cast<float>( video.xres() ) - 0.5f ) / _world->zoom + _world->camera.x ) / CHUNKSIZE;
  const float mY = ( CHUNKSIZE * 4.0f * ( static_cast<float>( MouseY ) / static_cast<float>( video.yres() ) - 0.5f ) / _world->zoom + _world->camera.z ) / CHUNKSIZE;

  // draw brush
  glPushMatrix();
    glScalef(_world->zoom,_world->zoom,1.0f);
    glTranslatef(-_world->camera.x/CHUNKSIZE,-_world->camera.z/CHUNKSIZE,0);

    glColor4f(1.0f,1.0f,1.0f,0.5f);
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    textureBrush.getTexture()->bind();

    const float tRadius = textureBrush.getRadius()/CHUNKSIZE;// *_world->zoom;
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

  displayGUIIfEnabled();
}

void MapView::displayViewMode_3D()
{
  //! \note Select terrain below mouse, if no item selected or the item is map.
  if( !_world->IsSelection( eEntry_Model ) && !_world->IsSelection( eEntry_WMO ) && Settings::getInstance()->AutoSelectingMode )
  {
    doSelection( true );
  }

  video.set3D();

  _world->draw();

  displayGUIIfEnabled();
}

void MapView::display()
{
  //! \todo  Get this out or do it somehow else. This is ugly and is a senseless if each draw.
  if( Saving )
  {
    video.setTileMode();
    _world->saveMap();
    Saving=false;
  }

  switch( mViewMode )
  {
  case eViewMode_2D:
    displayViewMode_2D();
    break;

  case eViewMode_3D:
    displayViewMode_3D();
    break;
  }
}

void MapView::resizewindow()
{
  mainGui->resize();
}

void MapView::keyPressEvent (QKeyEvent* event)
{
  //! \todo Implement GUI stuff again?
//  if( LastClicked && LastClicked->keyPressEvent( event ) )
//    return;

  if (event->key() == Qt::Key_Shift)
    Environment::getInstance()->ShiftDown = true;

  if (event->key() == Qt::Key_Alt)
    Environment::getInstance()->AltDown = true;

  if (event->key() == Qt::Key_Control)
    Environment::getInstance()->CtrlDown = true;

  // movement
  if (event->key() == Qt::Key_W)
  {
      key_w = true;
      moving = 1.0f;
  }

  if (event->key() == Qt::Key_S)
    moving = -1.0f;

  if (event->key() == Qt::Key_A)
    strafing = -1.0f;

  if (event->key() == Qt::Key_D)
    strafing = 1.0f;

  if (event->key() == Qt::Key_E)
    updown = -1.0f;

  if (event->key() == Qt::Key_Q)
    updown = 1.0f;

  //! \todo FUCK.
/*
  // position correction with num pad
  if (event->key() == Qt::Key_KP8 )
    keyx = -1;

  if (event->key() == Qt::Key_KP2 )
    keyx = 1;

  if (event->key() == Qt::Key_KP6 )
    keyz = -1;

  if (event->key() == Qt::Key_KP4 )
    keyz = 1;

  if (event->key() == Qt::Key_KP1 )
    keyy = -1;

  if (event->key() == Qt::Key_KP3 )
    keyy = 1;

  if (event->key() == Qt::Key_KP7 )
    keyr = 1;

  if (event->key() == Qt::Key_KP9 )
    keyr = -1;
    */

  if (event->key() == Qt::Key_X)
    toggle_terrain_mode_window();

  if (event->key() == Qt::Key_I && event->modifiers() & Qt::ControlModifier)
    toggle_painting_mode (!Environment::getInstance()->paintMode);

//  NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);
//  NEW_TOGGLE_ACTION (rotation_randomization, tr ("Randomized rotation when copying"), SLOT (toggle_copy_rotation_randomization (bool)), 0, false);


  if (event->key() == Qt::Key_C)
    Environment::getInstance()->cursorType = (Environment::getInstance()->cursorType + 1) % 4;

  if (event->key() == Qt::Key_U)
    toggle_tile_mode();

  if (event->key() == Qt::Key_Y)
    cycle_brush_type();

  if (event->key() == Qt::Key_Tab)
    toggle_interface();


  if (event->key() == Qt::Key_F1 && event->modifiers() & Qt::ShiftModifier)
    toggle_terrain_texturing_mode();

  // fog distance or brush radius
  if (event->key() == Qt::Key_Plus)
    if( event->modifiers() & Qt::AltModifier )
    {
      increase_brush_size();
    }
    else if( event->modifiers() & Qt::ShiftModifier && ( !_world->HasSelection() || ( _world->HasSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk) )  )
      _world->fogdistance += 60.0f;// fog change only when no model is selected!
    else
    {
      //change selected model size
      keys=1;
    }

  if (event->key() == Qt::Key_Minus)
    if (event->modifiers() & Qt::AltModifier)
    {
      decrease_brush_size();
    }
    else if( event->modifiers() & Qt::ShiftModifier && ( !_world->HasSelection() || ( _world->HasSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk) )  )
      _world->fogdistance -= 60.0f; // fog change only when no model is selected!
    else
    {
      //change selected model sizesize
      keys=-1;
    }

  // doodads set
  if( event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9 )
  {
    if( _world->IsSelection( eEntry_WMO ) )
    {
      _world->GetCurrentSelection()->data.wmo->doodadset = event->key() - Qt::Key_0;
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
      switch (event->key())
      {
        case Qt::Key_1:
          movespd = 15.0f;
          break;

        case Qt::Key_2:
          movespd = 50.0f;
          break;

        case Qt::Key_3:
          movespd = 300.0f;
          break;

        case Qt::Key_4:
          movespd = 1000.0f;
          break;
      }
    }
    else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_6)
    {
      terrainMode = event->key() - Qt::Key_1;
      mainGui->guiToolbar->IconSelect( terrainMode );
    }
  }
}

void MapView::toggle_detail_info_window (bool value)
{
  mainGui->guidetailInfos->hidden (!value);
}

void MapView::toggle_terrain_mode_window()
{
  if(terrainMode==2)
    view_texture_palette( 0, 0 );
  else if(terrainMode==4)
    mainGui->ZoneIDBrowser->toggleVisibility();
}

void MapView::toggle_painting_mode (bool value)
{
  Environment::getInstance()->paintMode = value;
}

void MapView::invert_mouse_y_axis (bool value)
{
  mousedir = value ? 1.0f : -1.0f;
}

/*!
  \brief Delete the current selected model
*/
void MapView::delete_selected_object()
{
  if( _world->IsSelection( eEntry_WMO ) )
    _world->deleteWMOInstance( _world->GetCurrentSelection()->data.wmo->mUniqueID );
  else if( _world->IsSelection( eEntry_Model ) )
    _world->deleteModelInstance( _world->GetCurrentSelection()->data.model->d1 );
}

/*!
  \brief Paste a model
  Paste the current model stored in Environment::getInstance()->get_clipboard() at the cords of the selected model or chunk.
*/
void MapView::paste_object()
{
  if( _world->HasSelection() )
  {
    nameEntry lClipboard = Environment::getInstance()->get_clipboard();
    switch( _world->GetCurrentSelection()->type )
     {
      case eEntry_Model:
        _world->addModel( lClipboard, _world->GetCurrentSelection()->data.model->pos );
        break;
      case eEntry_WMO:
        _world->addModel( lClipboard, _world->GetCurrentSelection()->data.wmo->pos);
        break;
      case eEntry_MapChunk:
        _world->addModel( lClipboard, _world->GetCurrentSelection()->data.mapchunk->GetSelectionPosition() );
        break;
      default: break;
    }
  }
}

/*!
  \brief Copy selected model to clipboard
  Copy the selected m2 or WMO with getInstance()->set_clipboard()
*/
void MapView::copy_selected_object()
{
  if( _world->HasSelection() )
  {
    Environment::getInstance()->set_clipboard( _world->GetCurrentSelection() );
  }
}

void MapView::increase_moving_speed()
{
  movespd *= 2.0f;
}

void MapView::decrease_moving_speed()
{
  movespd *= 0.5f;
}

void MapView::save_minimap()
{
  //! \todo This needs to be actually done here, not deferred to next display().
  Saving = true;
}

void MapView::turn_around()
{
  ah += 180.0f;
}

void MapView::reset_selected_object_rotation()
{
  if( _world->IsSelection( eEntry_WMO ) )
  {
    _world->GetCurrentSelection()->data.wmo->resetDirection();
    _world->setChanged(_world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z);
  }
  else if( _world->IsSelection( eEntry_Model ) )
  {
    _world->GetCurrentSelection()->data.model->resetDirection();
    _world->setChanged(_world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z);
  }
}

void MapView::snap_selected_object_to_ground()
{
  if( _world->IsSelection( eEntry_WMO ) )
  {
    Vec3D t = Vec3D( _world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z, 0 );
    _world->GetVertex( _world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z, &t );
    _world->GetCurrentSelection()->data.wmo->pos = t;
    _world->setChanged(_world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z);

  }
  else if( _world->IsSelection( eEntry_Model ) )
  {
    Vec3D t = Vec3D( _world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z, 0 );
    _world->GetVertex( _world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z, &t );
    _world->GetCurrentSelection()->data.model->pos = t;
    _world->setChanged(_world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z);
  }
}

void MapView::toggle_doodad_spawner()
{
  _doodad_spawner->hidden (!_doodad_spawner->hidden());
}

void MapView::toggle_lighting (bool value)
{
  _world->lighting = value;
}

void MapView::toggle_interface()
{
  _GUIDisplayingEnabled = !_GUIDisplayingEnabled;
}

void MapView::increase_time_speed()
{
  mTimespeed += 90.0f;
}
void MapView::decrease_time_speed()
{
  mTimespeed -= 90.0f;
}

void MapView::toggle_terrain_texturing_mode()
{
  if( alloff )
  {
    alloff_models = _world->drawmodels;
    alloff_doodads = _world->drawdoodads;
    alloff_contour = DrawMapContour;
    alloff_wmo = _world->drawwmo;
    alloff_fog = _world->drawfog;
    alloff_terrain = _world->drawterrain;

    _world->drawmodels = false;
    _world->drawdoodads = false;
    DrawMapContour = true;
    _world->drawwmo = false;
    _world->drawterrain = true;
    _world->drawfog = false;
  }
  else
  {
    _world->drawmodels = alloff_models;
    _world->drawdoodads = alloff_doodads;
    DrawMapContour = alloff_contour;
    _world->drawwmo = alloff_wmo;
    _world->drawterrain = alloff_terrain;
    _world->drawfog = alloff_fog;
  }
  alloff = !alloff;
}

void MapView::toggle_doodad_drawing (bool value)
{
  _world->drawmodels = value;
}

void MapView::toggle_auto_selecting (bool value)
{
  Settings::getInstance()->AutoSelectingMode = value;
}

void MapView::toggle_water_drawing (bool value)
{
  _world->drawwater = value;
}

void MapView::toggle_terrain_drawing (bool value)
{
  _world->drawterrain = value;
}

void MapView::toggle_wmo_doodad_drawing (bool value)
{
  _world->drawdoodads = value;
}

//! \todo these should be symetrical, so maybe combine.
void MapView::increase_brush_size()
{
  switch( terrainMode )
  {
  case 0:
    groundBrushRadius = qBound (0.0005f, groundBrushRadius + 0.01f, 1000.0f);
    ground_brush_radius->setValue( groundBrushRadius / 1000 );
    break;
  case 1:
    blurBrushRadius = qBound (0.01f, blurBrushRadius + 0.01f, 1000.0f);
    blur_brush->setValue( blurBrushRadius / 1000 );
    break;
  case 2:
    textureBrush.setRadius (qBound (0.1f, textureBrush.getRadius() + 0.1f, 100.0f));
    paint_brush->setValue( textureBrush.getRadius() / 100 );
    break;
  }
}

void MapView::decrease_brush_size()
{
  switch( terrainMode )
  {
  case 0:
    groundBrushRadius = qBound (0.0005f, groundBrushRadius - 0.01f, 1000.0f);
    ground_brush_radius->setValue( groundBrushRadius / 1000 );
    break;
  case 1:
    blurBrushRadius = qBound (0.01f, blurBrushRadius - 0.01f, 1000.0f);
    blur_brush->setValue( blurBrushRadius / 1000 );
    break;
  case 2:
    textureBrush.setRadius (qBound (0.1f, textureBrush.getRadius() - 0.1f, 100.0f));
    paint_brush->setValue( textureBrush.getRadius() / 100 );
    break;
  }
}

void MapView::toggle_hole_line_drawing (bool value)
{
  Environment::getInstance()->view_holelines = value;
}

void MapView::toggle_line_drawing (bool value)
{
  _world->drawlines = value;
}

void MapView::toggle_wmo_drawing (bool value)
{
  _world->drawwmo = value;
}

void MapView::toggle_minimap (bool value)
{
  _minimap->setVisible (value);
}

void MapView::save_all()
{
  _world->saveChanged();
}

void MapView::save()
{
  _world->saveTile( static_cast<int>( _world->camera.x ) / TILESIZE, static_cast<int>( _world->camera.z ) / TILESIZE );
}

void MapView::reload_current_tile()
{
  _world->reloadTile( static_cast<int>( _world->camera.x ) / TILESIZE, static_cast<int>( _world->camera.z ) / TILESIZE );
}

void MapView::exit_to_menu()
{
  close();
}

void MapView::cycle_brush_type()
{
  // toogle between smooth / flat / linear
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
}

void MapView::toggle_contour_drawing (bool value)
{
  DrawMapContour = value;
}

void MapView::toggle_fog_drawing (bool value)
{
  _world->drawfog = value;
}

void MapView::toggle_tile_mode()
{
  if( mViewMode == eViewMode_2D )
  {
    mViewMode = eViewMode_3D;
    terrainMode = saveterrainMode;
  }
  else
  {
    mViewMode = eViewMode_2D;
    saveterrainMode = terrainMode;
    terrainMode = 2;
  }

  // Set the right icon in toolbar
  mainGui->guiToolbar->IconSelect( terrainMode );
}

struct BookmarkEntry
{
  int map_id;
  int area_id;
  Vec3D position;
  float rotation;
  float tilt;
};

void MapView::add_bookmark()
{
  QSettings settings;

  //! \todo This MUST be easier and not needing to read and insert everything.
  QList<BookmarkEntry> bookmarks;

  int bookmarks_count (settings.beginReadArray ("bookmarks"));
  for (int i (0); i < bookmarks_count; ++i)
  {
    settings.setArrayIndex (i);

    BookmarkEntry b;
    b.map_id = settings.value ("map_id").toInt();
    b.position.x = settings.value ("camera/position/x").toFloat();
    b.position.y = settings.value ("camera/position/y").toFloat();
    b.position.z = settings.value ("camera/position/z").toFloat();
    b.rotation = settings.value ("camera/rotation").toFloat();
    b.tilt = settings.value ("camera/tilt").toFloat();
    b.area_id = settings.value ("area_id").toInt();

    bookmarks.append (b);
  }
  settings.endArray();

  BookmarkEntry new_bookmark;
  new_bookmark.map_id = _world->getMapID();
  new_bookmark.area_id = _world->getAreaID();
  new_bookmark.position = Vec3D (_world->camera.x, _world->camera.y, _world->camera.z);
  new_bookmark.rotation = ah;
  new_bookmark.tilt = av;

  bookmarks.append (new_bookmark);

  settings.beginWriteArray ("bookmarks");
  for (int i (0); i < bookmarks.size(); ++i)
  {
    settings.setArrayIndex (i);

    settings.setValue ("map_id", bookmarks[i].map_id);
    settings.setValue ("camera/position/x", bookmarks[i].position.x);
    settings.setValue ("camera/position/y", bookmarks[i].position.y);
    settings.setValue ("camera/position/z", bookmarks[i].position.z);
    settings.setValue ("camera/rotation", bookmarks[i].rotation);
    settings.setValue ("camera/tilt", bookmarks[i].tilt);
    settings.setValue ("area_id", bookmarks[i].area_id);
  }
  settings.endArray();

  //! \todo Signal the change of settings somehow, so Menu can update.
}

void MapView::keyReleaseEvent (QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
  {
    Environment::getInstance()->ShiftDown = false;
  }

  if (event->key() == Qt::Key_Alt)
  {
    Environment::getInstance()->AltDown = false;
  }

  if (event->key() == Qt::Key_Control)
  {
    Environment::getInstance()->CtrlDown = false;
  }

  // movement
  if (event->key() == Qt::Key_W)
  {
    key_w = false;
    if( !(leftMouse && rightMouse) && moving > 0.0f) moving = 0.0f;
  }

  if (event->key() == Qt::Key_S && moving < 0.0f )
    moving = 0.0f;

  if (event->key() == Qt::Key_D && strafing > 0.0f )
    strafing = 0.0f;

  if (event->key() == Qt::Key_A && strafing < 0.0f )
    strafing = 0.0f;

  if (event->key() == Qt::Key_Q && updown > 0.0f )
    updown = 0.0f;

  if (event->key() == Qt::Key_E && updown < 0.0f )
    updown = 0.0f;

  //! \todo FUCK.
/*
  if (event->key() == Qt::Key_KP8 )
    keyx = 0;

  if (event->key() == Qt::Key_KP2 )
    keyx = 0;

  if (event->key() == Qt::Key_KP6 )
    keyz = 0;

  if (event->key() == Qt::Key_KP4 )
    keyz = 0;

  if (event->key() == Qt::Key_KP1 )
    keyy = 0;

  if (event->key() == Qt::Key_KP3 )
    keyy = 0;

  if (event->key() == Qt::Key_KP7 )
    keyr = 0;

  if (event->key() == Qt::Key_KP9 )
    keyr = 0;

  if (event->key() == Qt::Key_KP_MINUS || e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
    keys = 0;
    */
}

void MapView::mousePressEvent (QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    leftMouse = true;
  }
  if (event->button() == Qt::RightButton)
  {
    rightMouse = true;
  }
  if (event->button() == Qt::MidButton)
  {
    if (_world->HasSelection())
    {
      MoveObj = true;
    }
  }

  if (leftMouse && rightMouse)
  {
   // Both buttons
   moving = 1.0f;
  }
  else if (leftMouse)
  {
    LastClicked = mainGui->processLeftClick (event->x(), event->y());
    if (mViewMode == eViewMode_3D && !LastClicked)
    {
      doSelection (false);
    }
  }
  else if (rightMouse)
  {
    look = true;
  }
}

void MapView::mouseReleaseEvent (QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    leftMouse = false;

    if( LastClicked )
      LastClicked->processUnclick();

    if(!key_w && moving > 0.0f )
      moving = 0.0f;

    if( mViewMode == eViewMode_2D )
    {
      strafing = 0;
      moving = 0;
    }
  }
  if (event->button() == Qt::RightButton)
  {
    rightMouse = false;

    look = false;

    if(!key_w && moving > 0.0f )moving = 0.0f;

    if( mViewMode == eViewMode_2D )
    {
      updown = 0;
    }
  }
  if (event->button() == Qt::MidButton)
  {
    MoveObj = false;
  }
}

void MapView::mouseMoveEvent (QMouseEvent* event)
{
  const QPoint relative_move (event->pos() - _last_drag_position);

  if (look && event->modifiers() == Qt::NoModifier)
  {
    ah += relative_move.x() / XSENS;
    av += mousedir * relative_move.y() / YSENS;

    av = qBound (-80.0f, av, 80.0f);
  }

  if( MoveObj )
  {
    mh = -video.ratio() * relative_move.x() / static_cast<float>( video.xres() );
    mv = -relative_move.y() / static_cast<float>( video.yres() );
  }
  else
  {
    mh = 0.0f;
    mv = 0.0f;
  }

  if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier))
  {
    rh = relative_move.x() / XSENS * 5.0f;
    rv = relative_move.y() / YSENS * 5.0f;
  }

  if (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::AltModifier)
  {
    switch( terrainMode )
    {
    case 0:
      groundBrushRadius += relative_move.x() / XSENS;
      if( groundBrushRadius > 1000.0f )
        groundBrushRadius = 1000.0f;
      else if( groundBrushRadius < 0.01f )
        groundBrushRadius = 0.01f;
      ground_brush_radius->setValue( groundBrushRadius / 1000 );
      break;
    case 1:
      blurBrushRadius += relative_move.x() / XSENS;
      if( blurBrushRadius > 1000.0f )
        blurBrushRadius = 1000.0f;
      else if( blurBrushRadius < 0.01f )
        blurBrushRadius = 0.01f;
      blur_brush->setValue( blurBrushRadius / 1000 );
      break;
    case 2:
      textureBrush.setRadius( textureBrush.getRadius() + relative_move.x() / XSENS );
      if( textureBrush.getRadius() > 100.0f )
        textureBrush.setRadius(100.0f);
      else if( textureBrush.getRadius() < 0.1f )
        textureBrush.setRadius(0.1f);
      paint_brush->setValue( textureBrush.getRadius() / 100.0f );
      break;
    }
  }

  if (event->buttons() & Qt::LeftButton && LastClicked)
  {
    LastClicked->processLeftDrag( event->x() - 4, event->y() - 4, relative_move.x(), relative_move.y() );
  }

  if( mViewMode == eViewMode_2D && event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ShiftModifier)
  {
    strafing = ((relative_move.x() / XSENS) / -1.0f) * 5.0f;
    moving = (relative_move.y() / YSENS) * 5.0f;
  }

  if( mViewMode == eViewMode_2D && event->buttons() & Qt::RightButton && event->modifiers() & Qt::ShiftModifier)
  {
    updown = (relative_move.y() / YSENS);
  }

  Environment::getInstance()->screenX = MouseX = event->x();
  Environment::getInstance()->screenY = MouseY = event->y();

  _last_drag_position = event->pos();
}
