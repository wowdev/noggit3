#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>

#include "menu.h"
#include "mpq.h"
#include "MapView.h"
#include "dbcfile.h"
#include "dbc.h"
#include "Log.h"
#include "world.h"

extern Directory * gFileList;

using namespace std;

/// TODO: Take this out.
/*TreeView * tv;
void TVSelectFunction( std::string pFile )
{
	gLog( "Selected: %s\n", pFile.c_str() );
}*/

extern std::list<std::string> gListfile;

Menu::Menu( )
{
	sel = -1;
	cmd = CMD_SELECT;
	world = 0;

	mt = 0;

	setpos = true;
	ah = -90.0f;
	av = -30.0f;

	lastbg = -1;

	int x = 5, y = 74, size;

	for( int type = 0; type < 3; type++ )
	{
		for( DBCFile::Iterator i = gMapDB.begin( ); i != gMapDB.end( ); ++i ) 
		{
			if( y > video.yres - 32 )
			{
				y = 74;
				x += 190;
			}

			MapEntry e;

			int id = i->getInt( MapDB::MapID );
			e.name = i->getString( MapDB::InternalName );
			e.description = i->getLocalizedString( MapDB::Name );
			e.loadingscreen = i->getUInt( MapDB::LoadingScreen );

			if ( id == gGoto.mapid || e.name == gGoto.mapname )
			{
				try
				{
					if( gGoto.mapid != -1 )
						gGoto.mapname = gMapDB.getByID( gGoto.mapid ).getString( MapDB::InternalName );

					world = new World( gGoto.mapname.c_str( ) );
					world->camera = Vec3D( gGoto.x, gGoto.y, gGoto.z );
					cmd = CMD_LOAD_WORLD;
					gGoto.mapid = -1;
					gGoto.mapname = "NONE";
				}
				catch( ... )
				{
					LogError << "Did not find matching map for id " << gGoto.mapid << ". Aborting.." << std::endl;
				}
			}

			if( i->getInt( MapDB::AreaType ) != type || !IsEditableWorld( id ) )
				continue;

			e.x0 = x;
			e.y0 = y;

			if( e.name == "Azeroth" || e.name == "Kalimdor" || e.name == "Expansion01" || e.name == "Northrend" ) 
			{
				size = 24;
				e.font = arial24;
			}
			else
			{
				size = 16;
				e.font = arial16;
			}
			y += size;

			e.x1 = e.x0 + freetype::width( e.font, e.name.c_str( ) );
			e.y1 = e.y0 + size;

			maps.push_back( e );
		}
		y += 40;
	}
	
	randBackground( );
	refreshBookmarks( );

	/// TODO: Take this out.
/*	tv = new TreeView( 600, 10, gFileList, 0, TVSelectFunction );
	tv->Expand( );*/
}

void Menu::randBackground( )
{	
	std::vector<std::string> ui;
	ui.push_back( "BloodElf" );
	ui.push_back( "Deathknight" );
	ui.push_back( "Draenei" );
	ui.push_back( "Dwarf" );
	ui.push_back( "Human" );
	ui.push_back( "MainMenu" );
	ui.push_back( "MainMenu_BurningCrusade" );
	ui.push_back( "MainMenu_Northrend" );
	ui.push_back( "NightElf" );
	ui.push_back( "Orc" );
	ui.push_back( "Scourge" );
	ui.push_back( "Tauren" );

	int randnum;
	do
		randnum = randint( 0, ui.size( ) - 1 );
	while( randnum == lastbg );

	lastbg = randnum;

	std::stringstream filename;
	filename << "Interface\\Glues\\Models\\UI_" << ui[randnum] << "\\UI_" << ui[randnum] << ".m2";
	
    bg.reset( new Model( filename.str( ) ) );
	bg->ind = true;
}

void Menu::tick( float t, float dt )
{
	mt += dt * 1000.0f;
	globalTime = int( mt );

	if( bg )
		bg->updateEmitters( dt );
	else 
		randBackground( );

	if( cmd == CMD_DO_LOAD_WORLD ) 
	{
		if( video.fullscreen ) 
			SDL_ShowCursor( SDL_DISABLE );

		gWorld = world;

		world->initDisplay( );
		// calc coordinates

		if( setpos )
		{
			cz = 0;
			cx = 0;

			if( world->nMaps > 0 )
			{

				float fx = ( x / 12.0f );
				float fz = ( y / 12.0f );

				cx = int( fx );
				cz = int( fz );

				world->camera = Vec3D( fx * TILESIZE, 0.0f, fz * TILESIZE );
				world->autoheight = true;
			} 
			else 
			{
				Vec3D p;
				if( world->mHasAGlobalWMO ) 
					p = world->mWMOInstances.begin( )->second.pos;
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

		world->enterTileInit( cx, cz );
		
		MapView *t = new MapView( world, ah, av );

		gStates.push_back( t );

		sel = -1;
		world = 0;

		cmd = CMD_BACK_TO_MENU;
		bg.reset( );
	}
	else if( cmd == CMD_BACK_TO_MENU )
	{
		if( video.fullscreen )
			SDL_ShowCursor(SDL_ENABLE);
		
		cmd = CMD_SELECT;
		setpos = true;
		gWorld = 0;

		refreshBookmarks( );
        randBackground( );
	}
}

void Menu::display(float t, float dt)
{
	video.clearScreen();
	glDisable(GL_FOG);
	//video.set3D();
	if (bg) {
		Vec4D la(0.1f,0.1f,0.1f,1);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, la);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor4f(1,1,1,1);
		for (int i=0; i<8; i++) {
			GLuint light = GL_LIGHT0 + i;
			glLightf(light, GL_CONSTANT_ATTENUATION, 0);
			glLightf(light, GL_LINEAR_ATTENUATION, 0.7f);
			glLightf(light, GL_QUADRATIC_ATTENUATION, 0.03f);
			glDisable(light);
		}
			//glEnableClientState(GL_VERTEX_ARRAY);
			//glEnableClientState(GL_NORMAL_ARRAY);
			//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			//glEnable(GL_TEXTURE_2D);
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

	/* what was the sense of this?
	if (darken) {
		glDisable(GL_TEXTURE_2D);
		// background is too light so tame it down a bit
		glColor4f(0,0,0,0.35f);
		glBegin(GL_QUADS);
		glVertex2i(0,0);
		glVertex2i(video.xres,0);
		glVertex2i(video.xres,video.yres);
		glVertex2i(0,video.yres);
		glEnd();
	}*/

	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);
	int basex = 200;
	int basey = 0;
	int tilesize = 12;
	
	if (cmd==CMD_LOAD_WORLD) 
	{
		try 
		{
			DBCFile::Record screen = gLoadingScreensDB.getByID( maps[(sel != -1?sel:25)].loadingscreen );

			glEnable( GL_TEXTURE_2D );
			glColor4f(1,1,1,1);
			glBindTexture(GL_TEXTURE_2D, video.textures.add( screen.getString( LoadingScreensDB::Path ) ));
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
		if ((sel != -1) && (world!=0)) 
		{
			if (world->minimap) 
			{
				// minimap time! ^_^
				const int len = 768;
				glColor4f(1,1,1,1);
				glBindTexture(GL_TEXTURE_2D, world->minimap);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex2i(basex,basey);
				glTexCoord2f(1,0);
				glVertex2i(basex+len,basey);
				glTexCoord2f(1,1);
				glVertex2i(basex+len,basey+len);
				glTexCoord2f(0,1);
				glVertex2i(basex,basey+len);
				glEnd();
			}

			glDisable( GL_TEXTURE_2D );
			for( int j = 0; j < 64; j++ ) 
			{
				for( int i=0; i < 64; i++ ) 
				{
					if( world->maps[j][i] ) 
					{
						glColor4f( 0.7f, 0.9f, 0.8f, 0.2f );
						glBegin( GL_QUADS );
						glVertex2i( basex + i * tilesize, basey + j * tilesize );
						glVertex2i( basex + ( i + 1 ) * tilesize, basey + j * tilesize );
						glVertex2i( basex + ( i + 1 ) * tilesize, basey + ( j + 1 ) * tilesize );
						glVertex2i( basex + i * tilesize, basey + ( j + 1 ) * tilesize );
						glEnd( );
					}
				}
			}
			glEnable( GL_TEXTURE_2D );

			if( world->nMaps == 0 ) 
			{
				freetype::shprint( arial16, video.xres/2.0f-freetype::width( arial16, "Click to enter" )/2.0f, video.yres/2.0f, "Click to enter" );
			} 
			else 
			{
				freetype::shprint( arial16, video.xres/2.0f-freetype::width( arial16, "Select your starting point" )/2.0f, 10, "Select your starting point" );
			}
		} 
		else 
		{
			freetype::shprint( morpheus40, video.xres/2.0f-freetype::width( morpheus40, "Nogg-It - A WoW Map Editor" )/2.0f, 1.0f, "Nogg-It - A WoW Map Editor" );
			freetype::shprint( arial16, video.xres - 20 - freetype::width( arial16, APP_VERSION ), 10, APP_VERSION );
			freetype::shprint( arial16, video.xres - 20 - freetype::width( arial16, APP_DATE ), 25, APP_DATE );

			freetype::shprint( arial16, video.xres/2.0f-freetype::width( arial16, "Ufoz [...], Cryect, Beket, Schlumpf, Tigurius, Steff" )/2.0f-5, video.yres - 40, "Ufoz [...], Cryect, Beket, Schlumpf, Tigurius, Steff" );
			freetype::shprint( arial16, video.xres/2.0f-freetype::width( arial16, "World of Warcraft is (C) Blizzard Entertainment" )/2.0f-5, video.yres - 22, "World of Warcraft is (C) Blizzard Entertainment" );

			freetype::shprint( arial24, video.xres/2.0f-freetype::width( arial24, "Bookmarks" ), 74, "Bookmarks" );

			/// TODO: Take this out.
//			tv->render();

			for( unsigned int i = 0; i < bookmarks.size( ); i++ ) 
			{
				freetype::shprint( arial16, bookmarks[i].x0, bookmarks[i].y0, bookmarks[i].label.c_str( ) );
			}
		}

		for ( unsigned int i = 0; i < maps.size( ); i++ ) 
		{
			if( i == sel )
				freetype::shprinty( maps[i].font, maps[i].x0, maps[i].y0, maps[i].name.c_str( ) );
			else
				freetype::shprint( maps[i].font, maps[i].x0, maps[i].y0, maps[i].name.c_str( ) );
		}

		if ( sel != -1 ) 
		{
			freetype::shprint( arial32, video.xres/2.0f-freetype::width( arial32, maps[sel].description.c_str() )/2.0f-5, video.yres - 40, maps[sel].description.c_str() );
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
	}
}

void Menu::mousemove( SDL_MouseMotionEvent *e ) { } // virtual function needs to be implemented.

bool Clickable::hit( int x, int y )
{
	return (y >= y0) && (y < y1) && (x >= x0) && (x < x1);
}

void Menu::mouseclick( SDL_MouseButtonEvent *e )
{
	/// TODO: Take this out.
	// this is for the treeview test only. 
/*	if( e->type == SDL_MOUSEBUTTONDOWN ) 
	{
		if( e->button == SDL_BUTTON_LEFT )
		{
			tv->processLeftClick(e->x,e->y);
		}
	}*/

	if( e->state == SDL_RELEASED )	// We don't want to have it to unselect when releasing or something..
		return;
	if( cmd != CMD_SELECT ) 
		return;

	/// We are about entering a world
	if( sel != -1 && world != 0 && ( e->y < 12*64 ) && ( e->x > 200 ) ) 
	{
		x = e->x - 200;
		y = e->y;
		cmd = CMD_LOAD_WORLD;
		return;
	}

	int osel = sel;
	
	/// Did I hit a map?
	for( unsigned int i = 0; i < maps.size( ); i++ ) 
	{
		if ( maps[i].hit( e->x, e->y ) ) 
		{
			sel = i;
			if( sel != osel ) 
			{
				if( world != 0 ) 
					delete world;
				world = new World( maps[i].name.c_str( ) );
				return;
			}
		}
	}
	
	/// Did I hit a bookmark?
	for( unsigned int i = 0; i < bookmarks.size( ); i++ ) 
	{
		if( bookmarks[i].hit( e->x, e->y ) ) 
		{
			for ( unsigned int j = 0; j < maps.size( ); j++ ) 
			{
				if ( maps[j].name == bookmarks[i].basename ) 
				{
					sel = j;
					cmd = CMD_LOAD_WORLD;
					setpos = false;

					// setup camera, ah, av
					ah = bookmarks[i].ah;
					av = bookmarks[i].av;

					world = new World( bookmarks[i].basename.c_str( ) );
					world->camera = bookmarks[i].pos;

					cx = int(bookmarks[i].pos.x / TILESIZE);
					cz = int(bookmarks[i].pos.z / TILESIZE);

					return;
				}
			}
		}
	}

	/// I hit nothing. ._.
	if ( world != 0 ) 
		delete world;
	sel = -1;
	world = 0;
}

void Menu::refreshBookmarks()
{
	bookmarks.clear( );

	std::ifstream f( "bookmarks.txt" );
	if( !f.is_open( ) )
		// No bookmarks file
		return;

	int y = 110;
	const int x = video.xres/2.0f-130.0f;

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

		bookmarks.push_back( b );
	}
	f.close( );
}

