#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <ctime> 
#include <cstdlib> 
#include <iostream>

#include "menu.h"
#include "mpq.h"
#include "MapView.h"
#include "dbcfile.h"
#include "dbc.h"
#include "Log.h"
#include "world.h"
#include "TextureManager.h" // TextureManager, Texture
#include "noggit.h" // fonts, APP_*

#include "WMOInstance.h" // WMOInstance (only for loading WMO only maps, we never load..)

// ui classes
#include "frame.h" // frame
#include "statusBar.h" // statusBar
#include "menuBar.h" // menuBar, menu items, ..
#include "win_credits.h" // winCredits
#include "minimapWindowUI.h" // minimapWindowUI

//! \todo  Take this out.
/*
extern Directory * gFileList;
TreeView * tv;
void TVSelectFunction( const std::string& pFile )
{
	LogDebug << "Selected: " << pFile << std::endl;
}*/

extern std::list<std::string> gListfile;

Menu::Menu() : bg( NULL )
{
	sel = -1;
	newsel = -1;
	newbookmark = -1;
	cmd = CMD_SELECT;
	world = NULL;

	mt = 0;

	setpos = true;
	ah = -90.0f;
	av = -30.0f;

	lastbg = -1;

	// create frame for gui elements.
  guiFrame = new frame( 0.0f, 0.0f, video.xres, video.yres );

	minimap_x = 300;
	minimap_y = 70;

	minimap_win = new minimapWindowUI( minimap_x-10, minimap_y-10);
	guiFrame->addChild(minimap_win);

	mCredits = new winCredits();
	guiFrame->addChild(mCredits);

	// create and register menubar
	mbar = new menuBar();
	mbar->AddMenu( "File" );
	mbar->GetMenu( "File" )->AddMenuItemSwitch( "exit                                  ESC", &gPop, true );
	guiFrame->addChild( mbar );
	
  // create and register statusbar
	guiStatusbar = new statusBar( 0.0f, video.yres - 30.0f, video.xres, 30.0f );
	guiFrame->addChild(guiStatusbar);

	int x = 5, y = 74, fontsize;

	for( int type = 0; type < 3; type++ )
	{
		for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i ) 
		{
			if( y > video.yres - 32 )
			{
				y = 74;
				x += 190;
			}

			MapEntry e;

			int id = i->getInt( MapDB::MapID );
			e.mid = id;
			e.name = i->getString( MapDB::InternalName );
			e.description = i->getLocalizedString( MapDB::Name );
			e.loadingscreen = i->getUInt( MapDB::LoadingScreen );
			e.IsBattleground = i->getUInt( MapDB::IsBattleground );
			e.AreaType = i->getUInt( MapDB::AreaType );

			if( i->getInt( MapDB::AreaType ) != type || !IsEditableWorld( id ) )
				continue;

			e.x0 = x;
			e.y0 = y;

			if( e.name == "Azeroth" || e.name == "Kalimdor" || e.name == "Expansion01" || e.name == "Northrend" ) 
			{
				fontsize = 24;
				e.font = arial24;
			}
			else
			{
				fontsize = 16;
				e.font = arial16;
			}
			y += fontsize;

			e.x1 = e.x0 + freetype::width( e.font, e.name.c_str() );
			e.y1 = e.y0 + fontsize;

			maps.push_back( e );
		}
		y += 40;
	}
  
  
  static const char* typeToName[3] = { "Continent", "Dungeons", "Raid" };
  
	mbar->AddMenu( typeToName[0] );
	mbar->AddMenu( typeToName[1] );
	mbar->AddMenu( typeToName[2] );
  
  for( std::vector<MapEntry>::iterator it = maps.begin(); it != maps.end(); it++ )
  {
    mbar->GetMenu( typeToName[it->AreaType] )->AddMenuItemSet( it->name, &newsel, it->mid );
  }
  
	refreshBookmarks();
  
  static const size_t nBookmarksPerMenu = 20;
  const size_t nBookmarkMenus = ( bookmarks.size() / nBookmarksPerMenu ) + 1;
  
  if( nBookmarkMenus )
  {
    mbar->AddMenu( "Bookmarks" );
  }
  
  for( size_t i = 1; i < nBookmarkMenus; i++ )
  {
    std::stringstream name;
    name << "Bookmarks (" << ( i + 1 ) << ")";
    mbar->AddMenu( name.str() );
  }
  
  int n = 0;
  for( std::vector<Bookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); it++ )
  {
    std::stringstream name;
    const int page = ( n++ / nBookmarksPerMenu );
    if( page )
    {
      name << "Bookmarks (" << ( page + 1 ) << ")";
    }
    else
    {
      name << "Bookmarks";
    }
    mbar->GetMenu( name.str() )->AddMenuItemSet( it->basename, &newbookmark, it->mid );
  }
  
	randBackground();
  
	//! \todo  Take this out.
/*	tv = new TreeView( 600, 10, gFileList, 0, TVSelectFunction );
	tv->Expand();*/
}

Menu::~Menu()
{
  if( guiFrame )
  {
    delete guiFrame;
    guiFrame = NULL;
  }
  if( world )
  {
    delete world;
    world = NULL;
  }
  if( bg )
  {
    ModelManager::delbyname( bg->name );
    bg = NULL;
  }
}

void Menu::randBackground()
{
	// STEFF:TODO first need to fix m2 bg loading.
	// Noggit shows no ground if the deactivated gbs loaded
	// Alphamapping?
	std::vector<std::string> ui;
	//ui.push_back( "BloodElf" );		// No
	//ui.push_back( "Deathknight" );	// No
	//ui.push_back( "Draenei" );		// No
	ui.push_back( "Dwarf" );		    // Yes
	//ui.push_back( "Human" );			// No
	ui.push_back( "MainMenu" );			// Yes
	//ui.push_back( "NightElf" );		// No
	//ui.push_back( "Orc" );			// No
	//ui.push_back( "Scourge" );		// No
	//ui.push_back( "Tauren" );			// No

	int randnum;
	do
	{
		randnum = randint( 0, ui.size() - 1 );
	}
	while( randnum == lastbg );

	lastbg = randnum;

	std::stringstream filename;
	filename << "Interface\\Glues\\Models\\UI_" << ui[randnum] << "\\UI_" << ui[randnum] << ".m2";
  
  if( bg )
  {
    ModelManager::delbyname( bg->name );
    bg = NULL;
  }
  
	bg = reinterpret_cast<Model*>( ModelManager::items[ModelManager::add( filename.str() )] );
}

void Menu::tick( float t, float dt )
{
	mt += dt * 1000.0f;
	globalTime = int( mt );

	if( bg )
		bg->updateEmitters( mt );

	if( cmd == CMD_DO_LOAD_WORLD ) 
	{

		if( video.fullscreen ) 
			SDL_ShowCursor( SDL_DISABLE );

		gWorld = world;

		world->initDisplay();
		// calc coordinates

		if( setpos )
		{
			cz = 0;
			cx = 0;

			if( !world->mHasAGlobalWMO )
			{

				float fx = ( click_x / 12.0f );
				float fz = ( click_y / 12.0f );

				cx = int( fx );
				cz = int( fz );

				world->camera = Vec3D( fx * TILESIZE, 0.0f, fz * TILESIZE );
				world->autoheight = true;
			} 
			else 
			{
				Vec3D p;
				if( world->mHasAGlobalWMO ) 
					p = world->mWMOInstances.begin()->second.pos;
				else 
					p = Vec3D( 0.0f, 0.0f, 0.0f ); // empty map? :|
				
				cx = int( p.x / TILESIZE );
				cz = int( p.z / TILESIZE );

				world->camera = p + Vec3D( 0.0f, 25.0f, 0.0f );
			}
			world->lookat = world->camera + Vec3D( 0.0f, 0.0f, -1.0f );

			ah = -90.0f;
			av = -30.0f;
		}

		world->enterTile( cx, cz );
		
		gStates.push_back( new MapView( world, ah, av ) ); // on gPop, MapView is deleted.

		sel = -1;
		world = NULL; // world will be deleted in ~MapView

		cmd = CMD_BACK_TO_MENU;
		
		if( bg )
		{
      ModelManager::delbyname( bg->name );
      bg = NULL;
		}
	}
	else if( cmd == CMD_BACK_TO_MENU )
	{
		if( video.fullscreen )
			SDL_ShowCursor(SDL_ENABLE);
		
		resizewindow();

		cmd = CMD_SELECT;
		setpos = true;
		gWorld = NULL;
		sel = -1;
		newsel = -1;
		world = NULL;
    randBackground();
	}
}

void Menu::display(float t, float dt)
{
	if(newsel != sel)
	{
		this->mCredits->hidden=true;
		this->loadMap(newsel);
		newsel=sel;
	}
	
	if(newbookmark>0)
	{
		this->mCredits->hidden=true;
		this->minimap_win->hidden=true;
		this->loadBookmark(newbookmark);
		newbookmark=0;
	}

	video.clearScreen();
	glDisable(GL_FOG);

	if (bg) {
		Vec4D la(0.1f,0.1f,0.1f,1);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, la);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor4f(1,1,1,1);
		for (int i=0; i<8; i++) 
		{
			GLuint light = GL_LIGHT0 + i;
			glLightf(light, GL_CONSTANT_ATTENUATION, 0);
			glLightf(light, GL_LINEAR_ATTENUATION, 0.7f);
			glLightf(light, GL_QUADRATIC_ATTENUATION, 0.03f);
			glDisable(light);
		}

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_LIGHTING);
		bg->cam.setup(globalTime);
		bg->draw();
	}

	video.set2D();
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glColor4f(1,1,1,1);

	glDisable( GL_TEXTURE_2D );

	// should the minimap window get rendert?
	if (cmd==CMD_SELECT) 
		if ((sel != -1) && (world!=0)) 
			minimap_win->hidden=false;
		else minimap_win->hidden=true;
	else minimap_win->hidden=true;
	
	// Draw gui elements								
	guiFrame->render();


	glEnable(GL_TEXTURE_2D);

	int tilesize = 12;
	
	if (cmd==CMD_LOAD_WORLD) 
	{
		try 
		{
			DBCFile::Record screen = gLoadingScreensDB.getByID( maps[(sel != -1?sel:25)].loadingscreen );
	
			glEnable( GL_TEXTURE_2D );
			glColor4f(1,1,1,1);
			glBindTexture(GL_TEXTURE_2D, TextureManager::add( screen.getString( LoadingScreensDB::Path ) ));
			glBegin(GL_QUADS);	
			
			glTexCoord2f(0,0);
			glVertex2i(0,0);
			glTexCoord2f(1,0);
			glVertex2i(video.xres,0);
			glTexCoord2f(1,1);
			glVertex2i(video.xres,video.yres);
			glTexCoord2f(0,1);
			glVertex2i(0,video.yres);
			glEnd();
			glDisable(GL_TEXTURE_2D);
		}
		catch( ... )
		{
			LogError << "Did not find matching loading screen. Check map.dbc for the entry please." << std::endl;
		}

		freetype::shprint( arial32, video.xres/2.0f-freetype::width( arial32, "Loading..." )/2.0f, video.yres/2.0f, "Loading..." );
		cmd = CMD_DO_LOAD_WORLD;
	}
	else if (cmd==CMD_SELECT) 
	{

		//! \todo  Get this stuff into minimap_win
		if ((sel != -1) && (world!=0)) 
		{
			const int len = 768;

			glEnable( GL_TEXTURE_2D );

			if (world->minimap) 
			{
				
				// draw the WDL data for the whole 64x64 ADTs

				glColor4f(1,1,1,1);
				glBindTexture(GL_TEXTURE_2D, world->minimap);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex2i(minimap_x,minimap_y);
				glTexCoord2f(1,0);
				glVertex2i(minimap_x+len,minimap_y);
				glTexCoord2f(1,1);
				glVertex2i(minimap_x+len,minimap_y+len);
				glTexCoord2f(0,1);
				glVertex2i(minimap_x,minimap_y+len);
				glEnd();
			}

			// draw the ADTs that are existing in the WDT with
			// a transparent 11x11 box. 12x12 is the full size 
			// so we get a smale border. Draw all not existing adts with 
			// a lighter box to have all 64x64 possible adts on screen. 
			// Later we can create adts over this view ore move them.
			glDisable( GL_TEXTURE_2D );
			for( int j = 0; j < 64; j++ ) 
			{
				for( int i=0; i < 64; i++ ) 
				{
					if( world->hasTile(j,i) ) 
					{
						glColor4f( 0.8f, 0.8f, 0.8f, 0.4f );
						glBegin( GL_QUADS );
						glVertex2i( minimap_x + i * tilesize, minimap_y + j * tilesize );
						glVertex2i( (minimap_x + ( i + 1 ) * tilesize) - 1, minimap_y + j * tilesize );
						glVertex2i( (minimap_x + ( i + 1 ) * tilesize) - 1, (minimap_y + ( j + 1 ) * tilesize) -1 );
						glVertex2i( minimap_x + i * tilesize, (minimap_y + ( j + 1 ) * tilesize) -1 );
						glEnd();
					}
					else
					{
						glColor4f( 1, 1, 1, 0.05f );
						glBegin( GL_QUADS );
						glVertex2i( minimap_x + i * tilesize, minimap_y + j * tilesize );
						glVertex2i( (minimap_x + ( i + 1 ) * tilesize) - 1, minimap_y + j * tilesize );
						glVertex2i( (minimap_x + ( i + 1 ) * tilesize) - 1, (minimap_y + ( j + 1 ) * tilesize) -1 );
						glVertex2i( minimap_x + i * tilesize, (minimap_y + ( j + 1 ) * tilesize) -1 );
						glEnd();
					}
				}
			}
			glEnable( GL_TEXTURE_2D );
	
		} 

		if ( sel != -1 ) 
		{
			freetype::shprint( fritz16, video.xres/2.0f-freetype::width( fritz16, maps[sel].name.c_str() )/2.0f-5, video.yres - 25, maps[sel].name.c_str() );
		}
	}
}

void Menu::keypressed( SDL_KeyboardEvent *e )
{
	if( e->type == SDL_KEYDOWN ) 
	{
		if( e->keysym.sym == SDLK_ESCAPE ) 
		{
		    gPop = true;
		}
		if( e->keysym.sym == SDLK_l ) 
		{
			loadMap(1);	   
		}
	}
}

void Menu::mousemove( SDL_MouseMotionEvent *e ) { } // virtual function needs to be implemented.

bool Clickable::hit( int x, int y )
{
	return (y >= y0) && (y < y1) && (x >= x0) && (x < x1);
}

void Menu::mouseclick( SDL_MouseButtonEvent *e )
{
	//! \todo  Take this out.
	// this is for the treeview test only. 
/*	if( e->type == SDL_MOUSEBUTTONDOWN ) 
	{
		if( e->button == SDL_BUTTON_LEFT )
		{
			tv->processLeftClick(e->x,e->y);
		}
		}*/
	

	if( e->type == SDL_MOUSEBUTTONDOWN ) 
		if( e->button == SDL_BUTTON_LEFT )
		{
			guiFrame->processLeftClick( float( e->x ), float( e->y ) );
		}



	if( e->state == SDL_RELEASED )	// We don't want to have it to unselect when releasing or something..
		return;
	if( cmd != CMD_SELECT ) 
		return;

  /// We are about entering a world
  if( sel != -1 && world != 0 && ( e->y < 768+minimap_y ) && (e->y > minimap_y) && (e->x < 768+minimap_x) && ( e->x > minimap_x ) ) 
  {
    click_x = e->x - minimap_x;
    click_y = e->y - minimap_y;
    cmd = CMD_LOAD_WORLD;
    return;
  }
	
	this->mCredits->hidden = true;
  
	/*
	/// I hit nothing. ._.
	if ( world != 0 ) 
		delete world;
	sel = -1;
	newsel = -1;
	world = 0;
	minimap_win->hidden=true;
	*/
}

void Menu::resizewindow()
{
	for( std::vector<frame*>::iterator child = guiFrame->children.begin(); child != guiFrame->children.end(); ++child )
		if( (*child)->mustresize )
			(*child)->resize();
}

void Menu::loadMap( int mid )
{
	int osel = sel;
	for( unsigned int i = 0; i < maps.size(); i++ )
	{
		if ( maps[i].mid == mid ) 
		{
			sel = i;
			if( sel != osel ) 
			{
				if( world )
					delete world;
				world = new World( maps[i].name );
			}
		}
	}
}

void Menu::loadBookmark( int mid )
{
	for( unsigned int i = 0; i < bookmarks.size(); i++ ) 
	{
		if( bookmarks[i].mid == mid ) 
		{
			for ( unsigned int j = 0; j < maps.size(); j++ ) 
			{
				if ( maps[j].name == bookmarks[i].basename ) 
				{
					sel = j;
					cmd = CMD_LOAD_WORLD;
					setpos = false;

					// setup camera, ah, av
					ah = bookmarks[i].ah;
					av = bookmarks[i].av;

					world = new World( bookmarks[i].basename );
					world->camera = bookmarks[i].pos;

					cx = int(bookmarks[i].pos.x / TILESIZE);
					cz = int(bookmarks[i].pos.z / TILESIZE);
				}
			}
		}
	}
}

void Menu::refreshBookmarks()
{
	bookmarks.clear();
  
	std::ifstream f( "bookmarks.txt" );
	if( !f.is_open() )
  {
		LogDebug << "No bookmarks file." << std::endl;
		return;
  }

	int y = 110;
	const int x = video.xres/2.0f-130.0f;
	int count_id = 1;
	while ( !f.eof() ) 
	{
		Bookmark b;
		int areaID;
		f >> b.basename >> b.pos.x >> b.pos.y >> b.pos.z >> b.ah >> b.av >> areaID;
		if ( f.eof() ) 
			break;

		// check for the basename
		bool mapfound = false;
		for ( unsigned int i=0; i<maps.size(); i++ ) 
		{
			if ( maps[i].name == b.basename ) 
			{
				mapfound = true;
				break;
			}
		}

		if ( !mapfound )
			continue;

		std::stringstream temp;
		temp << b.basename << ": " << AreaDB::getAreaName( areaID );
		b.label = temp.str();

		b.x0 = x;
		b.x1 = x + freetype::width( arial16, b.label.c_str() );
		b.y0 = y;
		b.y1 = y+16;
		y += 16;
		b.mid = count_id;
		count_id++;
		bookmarks.push_back( b );
	}
	f.close();
}
