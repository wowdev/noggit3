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

extern std::list<std::string> gListfile;

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

  mGUIFrame = new UIFrame( 0.0f, 0.0f, video.xres, video.yres );
  mGUIMinimapWindow = new UIMinimapWindow( this  );
  mGUIFrame->addChild( mGUIMinimapWindow );
  mGUICreditsWindow = new UIAbout();
  mGUIFrame->addChild( mGUICreditsWindow );
  //! \todo Use? Yes - later i will show here the adt cords where you enter and some otehr infos
  mGUIStatusbar = new UIStatusBar( 0.0f, video.yres - 30.0f, video.xres, 30.0f );
  mGUIFrame->addChild( mGUIStatusbar );
  createMapList();
  createBookmarkList();
  buildMenuBar();
  randBackground();
}

Menu::~Menu()
{
  delete mGUIFrame;
  mGUIFrame = NULL;
  
  delete gWorld;
  gWorld = NULL;
  
  ModelManager::delbyname( mBackgroundModel->name() );
  mBackgroundModel = NULL;
}

void Menu::randBackground()
{
  // STEFF:TODO first need to fix m2 bg loading.
  // Noggit shows no ground if the deactivated gbs loaded
  // Alphamapping?
  std::vector<std::string> ui;
  //ui.push_back( "BloodElf" );    // No
  //ui.push_back( "Deathknight" );  // No
  //ui.push_back( "Draenei" );    // No
  ui.push_back( "Dwarf" );      // Yes
  //ui.push_back( "Human" );      // No
  ui.push_back( "MainMenu" );      // Yes
  //ui.push_back( "NightElf" );    // No
  //ui.push_back( "Orc" );      // No
  //ui.push_back( "Scourge" );    // No
  //ui.push_back( "Tauren" );      // No

  int randnum;
  do
  {
    randnum = misc::randint( 0, ui.size() - 1 );
  }
  while( randnum == mLastBackgroundId );

  mLastBackgroundId = randnum;

  std::stringstream filename;
  filename << "Interface\\Glues\\Models\\UI_" << ui[randnum] << "\\UI_" << ui[randnum] << ".m2";
  
  if( mBackgroundModel )
  {
    ModelManager::delbyname( mBackgroundModel->name() );
    mBackgroundModel = NULL;
  }
  
  mBackgroundModel = ModelManager::item( ModelManager::add( filename.str() ) );
  mBackgroundModel->mPerInstanceAnimation = true;
}


void Menu::enterMapAt( Vec3D pos, bool autoHeight, float av, float ah )
{
  Vec2D tile( pos.x / TILESIZE, pos.y / TILESIZE );
  
  gWorld->autoheight = autoHeight;
  
  gWorld->camera = Vec3D( pos.x, pos.y, pos.z );
  gWorld->lookat = Vec3D( pos.x, pos.y, pos.z - 1.0f );
  
  gWorld->initDisplay();
  gWorld->enterTile( tile.x, tile.y );
  
  gStates.push_back( new MapView(  ah, av ) ); // on gPop, MapView is deleted.
  randBackground();
}

void Menu::tick( float t, float /*dt*/ )
{
  globalTime = t;
  
  if( mBackgroundModel )
  {
    mBackgroundModel->updateEmitters( t );
  }

  mGUIMinimapWindow->hidden = !gWorld;
}

void Menu::display(float /*t*/, float /*dt*/)
{
  // 3D: Background.
  video.clearScreen();
  glDisable( GL_FOG );
  
  if( mBackgroundModel )
  {
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    
    Vec4D la( 0.1f, 0.1f, 0.1f, 1.0f );
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, la );
    
    glEnable( GL_COLOR_MATERIAL );
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    for( int i = 0; i < 8; ++i ) 
    {
      GLuint light = GL_LIGHT0 + i;
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
    
    glDisable(GL_TEXTURE_2D);
    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
  }
  
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
      mGUIMinimapWindow->hidden = true;
      delete gWorld;
      gWorld = NULL;
    }
    else
    {
      gPop = true;
    }
  }
}

void Menu::mouseclick( SDL_MouseButtonEvent* e )
{
  mGUICreditsWindow->hidden = true;
  
  if( e->type == SDL_MOUSEBUTTONDOWN && e->button == SDL_BUTTON_LEFT )
  {
    mGUIFrame->processLeftClick( e->x, e->y );
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
  
  size_t entryStringLengths[sizeof( typeToName ) / sizeof( const char*)];
  
  for( std::vector<MapEntry>::const_iterator it = mMaps.begin(); it != mMaps.end(); ++it )
  {
	  mGUImenuBar->GetMenu( typeToName[it->areaType] )->AddMenuItemButton( it->name, &showMap, it->mapID );
    entryStringLengths[it->areaType] = std::max( entryStringLengths[it->areaType], it->name.length() );
  }
  
  const float menuWidthPerCharacterFactor = 7.0f;

  for( size_t i = 0; i < sizeof( entryStringLengths ) / sizeof( size_t ); ++i )
  {
    mGUImenuBar->GetMenu( typeToName[i] )->width = menuWidthPerCharacterFactor * entryStringLengths[i];
  }
  
  static const size_t nBookmarksPerMenu = 20;
  const size_t nBookmarkMenus = ( mBookmarks.size() / nBookmarksPerMenu ) + 1;
  
  if( nBookmarkMenus )
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
    
    
    MenuPane* pane = mGUImenuBar->GetMenu( name.str() );
    pane->AddMenuItemButton( it->name, &showBookmark, n );
    pane->width = std::max( pane->width, menuWidthPerCharacterFactor * it->name.length() );
  }
}

void Menu::createMapList()
{
  for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i ) 
  {
    MapEntry e;
    e.mapID = i->getInt( MapDB::MapID );
    e.name = misc::replaceSpecialChars(i->getLocalizedString( MapDB::Name ));//now map's name snown correctly! in ruRU client there was corrupted symbols
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
  while ( f >> basename >> b.pos.x >> b.pos.y >> b.pos.z >> b.ah >> b.av >> areaID ) 
  {
    int mapID = -1;

    // check for the basename
    for( std::vector<MapEntry>::const_iterator it = mMaps.begin(); it != mMaps.end(); ++it )
    {
      if ( it->name == basename ) 
      {
        mapID = it->mapID;
        break;
      }
    }

    if( mapID == -1 )
      continue;

    std::stringstream temp;
    temp << basename << ": " << AreaDB::getAreaName( areaID );
    b.name = temp.str();
    b.mapID = mapID;
    mBookmarks.push_back( b );
  }
  f.close();
}
