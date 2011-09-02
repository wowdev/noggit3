#include "Menu.h"

#include <cstdlib> 
#include <ctime> 
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "DBC.h"
#include "DBCFile.h"
#include "Log.h"
#include "MapView.h"
#include "Misc.h"
#include "ModelManager.h" // ModelManager
#include "MPQ.h"
#include "Noggit.h" // fonts, APP_*
#include "TextureManager.h" // TextureManager, Texture
#include "UIAbout.h" // UIAbout
#include "UIFrame.h" // UIFrame
#include "UIMenuBar.h" // UIMenuBar, menu items, ..
#include "UIMinimapWindow.h" // UIMinimapWindow
#include "UIStatusBar.h" // UIStatusBar
#include "WMOInstance.h" // WMOInstance (only for loading WMO only maps, we never load..)
#include "World.h"
#include "Settings.h"

Menu* theMenu = NULL;

void showMap( UIFrame *, int mapID )
{
  if( theMenu )
  {
    theMenu->loadMap( mapID );
  }
}

void showBookmark( UIFrame *, int bookmarkID )
{
  if( theMenu )
  {
    theMenu->loadBookmark( bookmarkID );
  }
}

Menu::Menu()
: mGUIFrame( NULL )
, mGUIStatusbar( NULL )
, mGUICreditsWindow( NULL )
, mGUIMinimapWindow( NULL )
, mGUImenuBar( NULL )
, mBackgroundModel( NULL )
, mLastBackgroundId( -1 )
{
  gWorld = NULL;
  theMenu = this;

  mGUIFrame = new UIFrame( 0.0f, 0.0f, video.xres(), video.yres() );
  mGUIMinimapWindow = new UIMinimapWindow( this );
  mGUIMinimapWindow->hide();
  mGUIFrame->addChild( mGUIMinimapWindow );
  mGUICreditsWindow = new UIAbout();
  mGUIFrame->addChild( mGUICreditsWindow );
  //! \todo Use? Yes - later i will show here the adt cords where you enter and some otehr infos
  mGUIStatusbar = new UIStatusBar( 0.0f, video.yres() - 30.0f, video.xres(), 30.0f );
  mGUIFrame->addChild( mGUIStatusbar );
  
  createMapList();
  createBookmarkList();
  buildMenuBar();
  randBackground();
}

//! \todo Add TBC and WOTLK.
//! \todo Use std::array / boost::array.
//const std::string uiModels[] = { "BloodElf", "Deathknight", "Draenei", "Dwarf", "Human", "MainMenu", "NightElf", "Orc", "Scourge", "Tauren" };
//Steff: Turn of the ugly once
const std::string uiModels[] = { "Deathknight", "Draenei", "Dwarf",  "MainMenu", "NightElf", "Orc" };


std::string buildModelPath( size_t index )
{
  assert( index < sizeof( uiModels ) / sizeof( const std::string ) );
  
  return "Interface\\Glues\\Models\\UI_" + uiModels[index] + "\\UI_" + uiModels[index] + ".m2";
}

Menu::~Menu()
{
  delete mGUIFrame;
  mGUIFrame = NULL;
  
  delete gWorld;
  gWorld = NULL;
  
  if( mBackgroundModel )
  {
    ModelManager::delbyname( buildModelPath( mLastBackgroundId ) );
    mBackgroundModel = NULL;
  }
}

void Menu::randBackground()
{
  if( mBackgroundModel )
  {
    ModelManager::delbyname( buildModelPath( mLastBackgroundId ) );
    mBackgroundModel = NULL;
  }

  int randnum;
  do
  {
    randnum = misc::randint( 0, sizeof( uiModels ) / sizeof( const std::string ) - 1 );
  }
  while( randnum == mLastBackgroundId );

  mLastBackgroundId = randnum;
  
  mBackgroundModel = ModelManager::add( buildModelPath( randnum ) );
  mBackgroundModel->mPerInstanceAnimation = true;
}


void Menu::enterMapAt( Vec3D pos, bool autoHeight, float av, float ah )
{
  video.farclip( Settings::getInstance()->FarZ );
  Vec2D tile( pos.x / TILESIZE, pos.y / TILESIZE );
  
  gWorld->autoheight = autoHeight;
  
  gWorld->camera = Vec3D( pos.x, pos.y, pos.z );
  gWorld->lookat = Vec3D( pos.x, pos.y, pos.z - 1.0f );
  
  gWorld->initDisplay();
  gWorld->enterTile( tile.x, tile.y );
  
  gStates.push_back( new MapView( ah, av ) ); // on gPop, MapView is deleted.
  
  mGUIMinimapWindow->hide();
  
  if( mBackgroundModel )
  {
    ModelManager::delbyname( buildModelPath( mLastBackgroundId ) );
    mBackgroundModel = NULL;
  }
}

void Menu::tick( float t, float /*dt*/ )
{
  //Steff: Why do this not work. If i use the given tick time to set globalTime the menu models are not animated?
  //globalTime = t;
  globalTime++;
  
  if( mBackgroundModel )
  {
    mBackgroundModel->updateEmitters( t );
  }
  else
  {
    randBackground();
  }
}

void Menu::display( float /*t*/, float /*dt*/ )
{
  // 3D: Background.
  video.clearScreen();
  
  video.set3D();
  
  glDisable( GL_FOG );
  
  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
  
  Vec4D la( 0.1f, 0.1f, 0.1f, 1.0f );
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, la );
  
  glEnable( GL_COLOR_MATERIAL );
  glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
  for(OpenGL::Light light = GL_LIGHT0; light < GL_LIGHT0 + 8; ++light )
  {
    glLightf( light, GL_CONSTANT_ATTENUATION, 0.0f );
    glLightf( light, GL_LINEAR_ATTENUATION, 0.7f );
    glLightf( light, GL_QUADRATIC_ATTENUATION, 0.03f );
    glDisable( light );
  }
  
  glEnable( GL_CULL_FACE );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_LIGHTING );
  glEnable( GL_TEXTURE_2D );
  
  mBackgroundModel->cam.setup( globalTime );
  mBackgroundModel->draw();
  
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );
  
  // 2D: UI.
  
  video.set2D();
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      
  mGUIFrame->render();
}

void Menu::keypressed( SDL_KeyboardEvent* e )
{
  if( e->type == SDL_KEYDOWN && e->keysym.sym == SDLK_ESCAPE ) 
  {
    if( gWorld )
    {
      mGUIMinimapWindow->hide();
      delete gWorld;
      gWorld = NULL;
    }
    else
    {
      gPop = true;
    }
  }
}

UIFrame::Ptr LastClickedMenu = NULL;

void Menu::mouseclick( SDL_MouseButtonEvent* e )
{
  mGUICreditsWindow->hide();
  
  if( e->button != SDL_BUTTON_LEFT )
  {
    return;
  }
  
  if( e->type == SDL_MOUSEBUTTONDOWN )
  {
    LastClickedMenu = mGUIFrame->processLeftClick( e->x, e->y );
  }
  else
  {
    LastClickedMenu = NULL;
  }
}

void Menu::mousemove( SDL_MouseMotionEvent *e )
{
  if( LastClickedMenu )
  {
    LastClickedMenu->processLeftDrag( e->x - 4, e->y - 4, e->xrel, e->yrel );
  }
}

void Menu::resizewindow()
{
  mGUIFrame->resize();
}

void Menu::loadMap( int mapID )
{
  delete gWorld;
  gWorld = NULL;
  
  for( DBCFile::Iterator it = gMapDB.begin(); it != gMapDB.end(); ++it )
  {
    if( it->getInt( MapDB::MapID ) == mapID )
    {
      gWorld = new World( it->getString( MapDB::InternalName ) );
      mGUIMinimapWindow->show();
      return;
    }
  }
  
  LogError << "Map with ID " << mapID << " not found. Failed loading." << std::endl;
}

void Menu::loadBookmark( int bookmarkID )
{
  BookmarkEntry e = mBookmarks.at( bookmarkID );
  loadMap( e.mapID );
  enterMapAt( e.pos, false, e.av, e.ah );
}

void Menu::buildMenuBar()
{
  if( mGUImenuBar )
  {
    mGUIFrame->removeChild( mGUImenuBar );
    delete mGUImenuBar;
    mGUImenuBar = NULL;
  }
  
  mGUImenuBar = new UIMenuBar();
  mGUImenuBar->AddMenu( "File" );
  mGUImenuBar->GetMenu( "File" )->AddMenuItemSwitch( "exit ESC", &gPop, true );
  mGUIFrame->addChild( mGUImenuBar );
  
  static const char* typeToName[3] = { "Continent", "Dungeons", "Raid" };
  
  mGUImenuBar->AddMenu( typeToName[0] );
  mGUImenuBar->AddMenu( typeToName[1] );
  mGUImenuBar->AddMenu( typeToName[2] );

  for( std::vector<MapEntry>::const_iterator it = mMaps.begin(); it != mMaps.end(); ++it )
  {
	  mGUImenuBar->GetMenu( typeToName[it->areaType] )->AddMenuItemButton( it->name, &showMap, it->mapID );
  }
  
  static const size_t nBookmarksPerMenu = 20;
  const size_t nBookmarkMenus = ( mBookmarks.size() / nBookmarksPerMenu ) + 1;
  
  if( mBookmarks.size() )
  {
    mGUImenuBar->AddMenu( "Bookmarks" );
  }
  
  for( size_t i = 1; i < nBookmarkMenus; ++i )
  {
    std::stringstream name;
    name << "Bookmarks (" << ( i + 1 ) << ")";
    mGUImenuBar->AddMenu( name.str() );
  }
  
  int n = -1;
  for( std::vector<BookmarkEntry>::const_iterator it = mBookmarks.begin(); it != mBookmarks.end(); ++it )
  {
    std::stringstream name;
    const int page = ( ++n / nBookmarksPerMenu );
    if( page )
    {
      name << "Bookmarks (" << ( page + 1 ) << ")";
    }
    else
    {
      name << "Bookmarks";
    }

    mGUImenuBar->GetMenu( name.str() )->AddMenuItemButton( it->name, &showBookmark, n );
  }
}

void Menu::createMapList()
{
  for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i ) 
  {
    MapEntry e;
    e.mapID = i->getInt( MapDB::MapID );
    e.name = i->getLocalizedString( MapDB::Name );
    e.areaType = i->getUInt( MapDB::AreaType );
    if( e.areaType < 0 || e.areaType > 2 || !World::IsEditableWorld( e.mapID ) )
      continue;

    mMaps.push_back( e );
  }
}

void Menu::createBookmarkList()
{
  mBookmarks.clear();
  
  std::ifstream f( "bookmarks.txt" );
  if( !f.is_open() )
  {
    LogDebug << "No bookmarks file." << std::endl;
    return;
  }
  
  std::string basename;
  int areaID;
  BookmarkEntry b;
  int mapID = -1;
  while ( f >> mapID >> b.pos.x >> b.pos.y >> b.pos.z >> b.ah >> b.av >> areaID ) 
  {
    if( mapID == -1 )
      continue;

    std::stringstream temp;
    temp << MapDB::getMapName(mapID) << ": " << AreaDB::getAreaName( areaID );
    b.name = temp.str();
    b.mapID = mapID;
    mBookmarks.push_back( b );
  }
  f.close();
}
