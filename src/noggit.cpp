#ifdef _WIN32
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"glu32.lib")

#define NOMINMAX
#include <windows.h>
#include <winerror.h>
#endif

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <list>

#include "appstate.h"
#include "menu.h"
#include "MapView.h"

#include "mpq.h"
#include "video.h"
#include "directory.h"
#include "dbc.h"
#include "model.h"

//#include "shaders.h"
#include "liquid.h"

#include "ConfigFile.h"
#include "Log.h"

#include "Settings.h"		// In this singleton you can insert user settings. This object will later be serialized to disk (userpath)
#include "Project.h"		// This singleton holds later all settings for the current project. Will also be serialized to a selectable place on disk.
#include "Environment.h"	// This singleton holds all vars you dont must save. Like bools for display options. We should move all global stuff here to get it OOP!

#include "errorhandling.h"

#include "AsyncLoader.h"

bool fullscreen = false;



std::vector<AppState*> gStates;
bool gPop = false;

std::string wowpath;

extern std::list<std::string> gListfile;

float gFPS;

freetype::font_data arialn13,arial12,arial14,arial16,arial24,arial32,morpheus40,skurri32,fritz16;	

GotoInfo gGoto;
AsyncLoader* gAsyncLoader;


void getGamePath(bool pLoadFromConfig = false)
{
	// temp to store path from registry
	char temp[1024];

	// no use of configuration file
	if( !pLoadFromConfig )
	{
		#ifdef _WIN32
			HKEY key;
			DWORD t,s;
			LONG l;
			s = 1024;
			memset(temp,0,s);
			l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\Beta",0,KEY_QUERY_VALUE,&key);
			if (l != ERROR_SUCCESS)
				l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\PTR",0,KEY_QUERY_VALUE,&key);
			if (l != ERROR_SUCCESS)
				l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Blizzard Entertainment\\World of Warcraft",0,KEY_QUERY_VALUE,&key);
			if (l == ERROR_SUCCESS) 
			{
				l = RegQueryValueEx(key,"InstallPath",0,&t,(LPBYTE)temp,&s);
				RegCloseKey(key);
				wowpath = std::string( temp );
			}
		#else
			pLoadFromConfig=true;
			wowpath = "/Applications/World of Warcraft/";
		#endif
	}
	if( temp[0] == 0  || pLoadFromConfig )
	{
		if( FileExists( "NoggIt.conf" ) )
		{
			ConfigFile config( "NoggIt.conf" );
			config.readInto( wowpath, "Path" );
		}
	}
}
//Directory * gFileList;

int	nTimers;
int Timers[25];

void TimerStart()
{
	Timers[nTimers]=SDL_GetTicks();
	nTimers++;
}

int TimerStop()
{
	int endTime=SDL_GetTicks();
	nTimers--;
	return endTime-Timers[nTimers];
}

void CreateStrips();


int startUnittests()
{
// Start some Unittests

	Settings::getInstance( );
	Project::getInstance( );
	Environment::getInstance( );

	Project::getInstance()->setPath("H:\\Client333_enUS\\");
	// Set up log.

	InitLogging( );

	Log << APP_TITLE << " " << APP_VERSION << std::endl;

	if( !video.init( 800, 600, fullscreen ) )
	{
		LogError << "Initializing video failed." << std::endl;
		return -1;
	}
	SDL_WM_SetCaption( "Noggit Screen Unit Test", "noggit.ico" );	

	Log << "Init Screen" << std::endl;

	wowpath = "H:/Client333_enUS/";

	CreateStrips( );

	Log << "Creat Stripes" << std::endl;

	//libmpq__init( );
	
	Log << "Libmpq" << std::endl;

	// data
	std::vector<MPQArchive*> archives;
	std::vector<std::string> archiveNames;
	archiveNames.push_back( "common.MPQ" );
	archiveNames.push_back( "common-2.MPQ" ); 
	archiveNames.push_back( "expansion.MPQ" );
	archiveNames.push_back( "lichking.MPQ" );
	archiveNames.push_back( "patch.MPQ" );
	archiveNames.push_back( "patch-2.MPQ" );
	archiveNames.push_back( "patch-3.MPQ" );
	// locals
	archiveNames.push_back( "enUS/locale-enUS.MPQ" );
	archiveNames.push_back( "enUS/expansion-locale-enUS.MPQ" );
	archiveNames.push_back( "enUS/lichking-locale-enUS.MPQ" );
	archiveNames.push_back( "enUS/patch-enUS.MPQ" );
	archiveNames.push_back( "enUS/patch-enUS-2.MPQ" );
	archiveNames.push_back( "enUS/patch-enUS-3.MPQ" );

	for( size_t i = 0; i < archiveNames.size( ); i++ )
	{
		// load the hardcoded mpqs
		std::string path = wowpath;
		path.append( "Data/" ).append( archiveNames[i] );
		archives.push_back( new MPQArchive( path ) );
	}

		Log << "Archives added" << std::endl;

	// Opening DBCs
	OpenDBs( );
	Log << "Open DBs" << std::endl;
	// Initializing Fonts
	
	skurri32.initMPQ( "fonts\\SKURRI.TTF", 32 );
	fritz16.initMPQ( "fonts\\FRIZQT__.TTF", 16 );
	morpheus40.initMPQ( "fonts\\MORPHEUS.TTF", 40 );
	arialn13.initMPQ( "fonts\\arialn.TTF", 13 );
	arial12.init( "fonts/arial.ttf", 12 );
	arial14.init( "fonts/arial.ttf", 14 );
	arial16.init( "fonts/arial.ttf", 16 );
	arial24.init( "fonts/arial.ttf", 24 );
	arial32.init( "fonts/arial.ttf", 32 );
	Log << "Fonts added" << std::endl;

	//Lood Model
	Model * bg;

	std::stringstream filename;
	filename << "Interface\\Glues\\Models\\UI_Tauren\\UI_Tauren.m2";
	
    bg = new Model( filename.str( ) );

	float mt = 1000.0f; 
	Log << "Entering Main Loop" << std::endl;
	bool done = false;
	while( !done) 
	{
		SDL_Event event;
		while ( SDL_PollEvent(&event) ) {
			if ( event.type == SDL_QUIT ) {
				done = true;
			}
			else if ( event.type == SDL_MOUSEMOTION) {
				if(SDL_GetAppState()&SDL_APPMOUSEFOCUS)
				{
					// handle events.
				}
			}
			else if ( (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)&&(SDL_GetAppState()&SDL_APPINPUTFOCUS)) {
				
				if(event.button.type == SDL_MOUSEBUTTONUP)
				{
					// handle events.
					mt += 100.0f;
				}
				else if(SDL_GetAppState()&SDL_APPMOUSEFOCUS)
				{
					// handle events.
				}
			}
			else if ( (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
				if(SDL_GetAppState()&SDL_APPINPUTFOCUS)
				{
					// handle events.
				}
			}
			else if( event.type == SDL_VIDEORESIZE)
			{
				// reset the resolution in video object
				video.resize(event.resize.w,event.resize.h);
				// message to the aktive gui element
				if(SDL_GetAppState())
				{
					// handle events.
				}
			}
		}
		video.clearScreen();

		glDisable(GL_FOG);
		glColor4f(1,1,1,1);
		glEnable(GL_TEXTURE_2D);

		if (bg) 
		{
			bg->updateEmitters( mt );
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
			bg->cam.setup( mt );
			bg->draw();
		}



		video.flip();
	}
		Log << "Close video" << std::endl;
	
	delete bg;
	
	video.close();

	return 0;
}

int startNoggit( int argc, char *argv[] )
{
	Settings::getInstance( );
	Project::getInstance( );
	Environment::getInstance( );

	// Set up log.
	InitLogging( );

	Log << APP_TITLE << " " << APP_VERSION << std::endl;
	
	// Why should we load anything when there are missing files? ...

	//! \todo  Get this file from %WINDOWS%
#ifdef _WIN32
	bool lFontWindows = FileExists( "C:\\windows\\fonts\\arial.ttf" );
#else
	bool lFontWindows = false;
#endif
	bool lFontLocal = FileExists( "fonts/arial.ttf" );
	if( !lFontWindows && !lFontLocal )
	{
		Log << "Can not find arial.ttf. This is really weird if you have windows. Add the file to the noggit directory then!" << std::endl;
		return -1;
	}
	
	TimerStart();

	srand( time( 0 ) );

	int xres = 1100;
	int yres = 900;


	bool useConfig = false;

	int gowto = -1;

	// handle starting parameters
	for( int i = 1; i < argc; i++ ) 
	{
		if( !strcmp( argv[i], "-f" ) || !strcmp( argv[i], "-fullscreen" ) ) 
			fullscreen = true;
		else if( !strcmp( argv[i], "-c" ) || !strcmp( argv[i], "-config" ) ) 
			useConfig = true;
		else if( !strcmp( argv[i], "-g" ) || !strcmp( argv[i], "-goto" ) ) 
			gowto = i + 1;

		else if( !strcmp( argv[i], "-w" ) || !strcmp( argv[i], "-windowed" ) ) 
			fullscreen = false;
		else if (!strcmp(argv[i],"-1024") || !strcmp(argv[i],"-1024x768")) {
			xres = 1024;
			yres = 768;
		}
		else if (!strcmp(argv[i],"-800") || !strcmp(argv[i],"-800x600")) {
			xres = 800;
			yres = 600;
		}
		else if (!strcmp(argv[i],"-1280") || !strcmp(argv[i],"-1280x1024")) {
			xres = 1280;
			yres = 1024;
		}
		else if (!strcmp(argv[i],"-1280x960")) {
			xres = 1280;
			yres = 960;
		}
		else if (!strcmp(argv[i],"-1400") || !strcmp(argv[i],"-1400x1050")) {
			xres = 1400;
			yres = 1050;
		}
		else if (!strcmp(argv[i],"-1280x800")) {
			xres = 1280;
			yres = 800;
		}
		else if (!strcmp(argv[i],"-1600") || !strcmp(argv[i],"-1600x1200")) {
			xres = 1600;
			yres = 1200;
		}
		else if (!strcmp(argv[i],"-1920") || !strcmp(argv[i],"-1920x1200")) {
			xres = 1920;
			yres = 1200;
		}
		else if (!strcmp(argv[i],"-2048") || !strcmp(argv[i],"-2048x1536")) {
			xres = 2048;
			yres = 1536;
		}
	}
	
	if( !video.init( xres, yres, fullscreen ) )
	{
		LogError << "Initializing video failed." << std::endl;
		return -1;
	}

	SDL_WM_SetCaption( APP_TITLE, "noggit.bmp" );
	gGoto.mapid = -1;
	gGoto.mapname = "NONE";

  //! \todo Did anyone ever use this? We may want to get it out again.
	if( gowto != -1 )
	{
		if( argc <= gowto )
			return 0;

		char name[1000];
		if( *argv[gowto] == 'n' )
			sscanf( argv[gowto]+2, "%f,%f,%f:%s", &gGoto.x, &gGoto.y, &gGoto.z, &name );
		else
			sscanf( argv[gowto]+2, "%f,%f,%f:%i", &gGoto.x, &gGoto.y, &gGoto.z, &gGoto.mapid );
		
		gGoto.mapname = std::string( name );
	}

	TimerStart();	

	getGamePath( useConfig );

	Log << "Game path: " << wowpath << std::endl;

	if( Project::getInstance( )->getPath( ) == "" )
		Project::getInstance( )->setPath( wowpath );

	Log << "Project path: " << Project::getInstance( )->getPath( ) << std::endl;

	CreateStrips( );
  
  gAsyncLoader = new AsyncLoader();
  gAsyncLoader->start(1); //! \todo get the number of threads from the number of available cores.
	
	//libmpq__init( );

	std::vector<MPQArchive*> archives;
	std::vector<std::string> archiveNames;
	archiveNames.push_back( "common.MPQ" );
	archiveNames.push_back( "common-2.MPQ" ); 
	archiveNames.push_back( "expansion.MPQ" );
	archiveNames.push_back( "lichking.MPQ" );
	archiveNames.push_back( "patch.MPQ" );
	archiveNames.push_back( "patch-{number}.MPQ" );
	archiveNames.push_back( "patch-{character}.MPQ" );
	
	//archiveNames.push_back( "{locale}/backup-{locale}.MPQ" );	
	//archiveNames.push_back( "{locale}/base-{locale}.MPQ" );
	archiveNames.push_back( "{locale}/locale-{locale}.MPQ" );
	//archiveNames.push_back( "{locale}/speech-{locale}.MPQ" );
	archiveNames.push_back( "{locale}/expansion-locale-{locale}.MPQ" );
	//archiveNames.push_back( "{locale}/expansion-speech-{locale}.MPQ" );
	archiveNames.push_back( "{locale}/lichking-locale-{locale}.MPQ" );
	//archiveNames.push_back( "{locale}/lichking-speech-{locale}.MPQ" );
	archiveNames.push_back( "{locale}/patch-{locale}.MPQ" );
	archiveNames.push_back( "{locale}/patch-{locale}-{number}.MPQ" );
	archiveNames.push_back( "{locale}/patch-{locale}-{character}.MPQ" );

	const char * locales[] = { "enGB", "enUS", "deDE", "koKR", "frFR", "zhCN", "zhTW", "esES", "esMX", "ruRU" };
	const char * locale = "****";
	
	// Find locale, take first one.
	for( int i = 0; i < 10; i++ )
	{
		std::string path = wowpath;
		path.append( "Data/" ).append( locales[i] ).append( "/realmlist.wtf" );
		if( FileExists( path ) )
		{
			locale = locales[i];
			Log << "Locale: " << locale << std::endl;
			break;
		}
	}
	if( !strcmp( locale, "****" ) )
	{
		LogError << "Could not find locale directory. Be sure, that there is one containing the file \"realmlist.wtf\"." << std::endl;
		return -1;
	}

	//! \todo  This may be done faster. Maybe.
	for( size_t i = 0; i < archiveNames.size( ); i++ )
	{
		std::string path = wowpath;
		path.append( "Data/" ).append( archiveNames[i] );
		std::string::size_type location = std::string::npos;

		do
		{
			location = path.find( "{locale}" );
			if( location != std::string::npos )
			{
				path.replace( location, 8, locale );
			}
		} 
		while( location != std::string::npos );

		if( path.find( "{number}" ) != std::string::npos )
		{
			char temp[10];
			location = path.find( "{number}" );
			path.replace( location, 8, " " );
			for( int i = 2; i < 10; i++ )
			{
				sprintf( temp, "%i", i );
				path.replace( location, 1, std::string( temp ) );
				if( FileExists( path ) )
					archives.push_back( new MPQArchive( path, true ) );
			}
		}
		else if( path.find( "{character}" ) != std::string::npos  )
		{
			char temp[10];
			location = path.find( "{character}" );
			path.replace( location, 11, " " );
			for( char i = 'a'; i <= 'z'; i++ )
			{
				sprintf( temp, "%c", i );
				path.replace( location, 1, std::string( temp ) );
				if( FileExists( path ) )
					archives.push_back( new MPQArchive( path, true ) );
			}
		}
		else
			if( FileExists( path ) )
				archives.push_back( new MPQArchive( path, true ) );
	}

	LogDebug << "main: mpqs: " << TimerStop( ) << " ms" << std::endl;
	TimerStart( );	

	// sort listfiles.
	gListfile.sort( );
	gListfile.unique( );

	//! \todo  Get this out?
	//gFileList = new Directory( "root" );
	//size_t found;
	// This is an example with filter:
	/*
		std::vector<std::string>::iterator it;
		for( it = gListfile.begin(); it != gListfile.end(); ++it )
		{
			if( it->find( pFilter ) != std::string::npos )
			{
				found = it->find_last_of("/\\");
				if( found != std::string::npos )
					mDirectory->AddSubDirectory( it->substr(0,found) )->AddFile( it->substr(found+1) );
				else
					mDirectory->AddFile( *it );
			}
		}
	*/
	// This is an example for getting all files in the list.
	/*	std::list<std::string>::iterator it;
		for( it = gListfile.begin(); it != gListfile.end(); ++it )
		{
			found = it->find_last_of("/\\");
			if( found != std::string::npos )
				gFileList->AddSubDirectory( it->substr(0,found) )->AddFile( it->substr(found+1) );
			else
				gFileList->AddFile( *it );
		}
   */

	LogDebug << "main: directory: " << TimerStop() << " ms" << std::endl;
	
	TimerStart();	
	
	// Opening DBCs
	OpenDBs( );

	// Initializing Fonts
	skurri32.initMPQ( "fonts\\SKURRI.TTF", 32 );
	fritz16.initMPQ( "fonts\\FRIZQT__.TTF", 16 );
	morpheus40.initMPQ( "fonts\\MORPHEUS.TTF", 40 );
	arialn13.initMPQ( "fonts\\arialn.TTF", 13 );
	if( lFontWindows )
	{
		arial12.init( "C:\\windows\\fonts\\arial.ttf", 12 );
		arial14.init( "C:\\windows\\fonts\\arial.ttf", 14 );
		arial16.init( "C:\\windows\\fonts\\arial.ttf", 16 );
		arial24.init( "C:\\windows\\fonts\\arial.ttf", 24 );
		arial32.init( "C:\\windows\\fonts\\arial.ttf", 32 );
	}
	else
	{
		arial12.init( "fonts/arial.ttf", 12 );
		arial14.init( "fonts/arial.ttf", 14 );
		arial16.init( "fonts/arial.ttf", 16 );
		arial24.init( "fonts/arial.ttf", 24 );
		arial32.init( "fonts/arial.ttf", 32 );
	}
	
	float ftime;
	Uint32 t, last_t, frames = 0, time = 0, fcount = 0, ft = 0;
	AppState *as;
	gFPS = 0;

	LogDebug << "Creating Menu" << std::endl;

	Menu *m = new Menu( );
	as = m;

	gStates.push_back( as );
	
	LogDebug << "main: created menu: " << TimerStop() << " ms" << std::endl;

	if( video.mSupportShaders )
		loadWaterShader();
	else
		LogError << "Your GPU does not support ARB vertex programs (shaders). Sorry." << std::endl;

	bool done = false;
	t = SDL_GetTicks( );

	LogDebug << "Entering Main Loop" << std::endl;

	while(gStates.size()>0 && !done) {
		last_t = t;
		t = SDL_GetTicks();
		Uint32 dt = t - last_t;
		time += dt;
		ftime = time / 1000.0f;

		as = gStates[gStates.size()-1];

		SDL_Event event;
		while ( SDL_PollEvent(&event) ) {
			if ( event.type == SDL_QUIT ) {
				done = true;
			}
			else if ( event.type == SDL_MOUSEMOTION) {
				if(SDL_GetAppState()&SDL_APPMOUSEFOCUS)
					as->mousemove(&event.motion);
			}
			else if ( (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)&&(SDL_GetAppState()&SDL_APPINPUTFOCUS)) {
				
				if(event.button.type == SDL_MOUSEBUTTONUP)
					as->mouseclick(&event.button);
				else if(SDL_GetAppState()&SDL_APPMOUSEFOCUS)
					as->mouseclick(&event.button);
			}
			else if ( (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
				if(SDL_GetAppState()&SDL_APPINPUTFOCUS)
					as->keypressed(&event.key);
			}
			else if( event.type == SDL_VIDEORESIZE)
			{
				// reset the resolution in video object
				video.resize(event.resize.w,event.resize.h);
				// message to the aktive gui element
				if(SDL_GetAppState())
					as->resizewindow();
			}
		}
		if(SDL_GetAppState()&SDL_APPACTIVE)
		{
				as->tick(ftime, dt/1000.0f);
				as->display(ftime, dt/1000.0f);
		}

		if (gPop) 
		{
			gPop = false;
			gStates.pop_back();
			delete as;
		}

		frames++;
		fcount++;
		ft += dt;
		if (ft >= 1000) 
		{
			gFPS = (float)fcount / (float)ft * 1000.0f;
		//	char buf[32];
		//	sprintf(buf, APP_TITLE " - %.2f fps",fps);
		//	SDL_WM_SetCaption(buf,NULL);
            ft = 0;
			fcount = 0;
		}

		video.flip();
	}
	
	video.close();

	for( std::vector<MPQArchive*>::iterator it = archives.begin( ); it != archives.end( ); ++it )
        (*it)->close();

	archives.clear();
	
	//libmpq__shutdown( );

	LogDebug << "Exited" << std::endl;

	return 0;
}

float frand()
{
    return rand()/(float)RAND_MAX;
}

float randfloat(float lower, float upper)
{
	return lower + (upper-lower)*(rand()/(float)RAND_MAX);
}

int randint(int lower, int upper)
{
    return lower + (int)((upper+1-lower)*frand());
}

int main( int argc, char *argv[] )
{
	RegisterErrorHandlers();
	
	#ifdef _UNITTEST
		// start UNITTESTS IF IN UNITTEST MODE
		startUnittests();
	#else
		// ELSE START APP
		startNoggit( argc, argv );
	#endif

	return 0;
}
