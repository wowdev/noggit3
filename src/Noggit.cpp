#include "Noggit.h"

#ifdef _WIN32
//#pragma comment(lib,"OpenGL32.lib")
//#pragma comment(lib,"glu32.lib")
#include <direct.h> 

#include <windows.h>
#include <winerror.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include <SDL.h>

#include "AppState.h"
#include "AsyncLoader.h"
#include "ConfigFile.h"
#include "DBC.h"
#include "Directory.h"
#include "Environment.h"  // This singleton holds all vars you dont must save. Like bools for display options. We should move all global stuff here to get it OOP!
#include "errorHandling.h"
#include "FreeType.h" // fonts.
#include "Liquid.h"
#include "Log.h"
#include "MapView.h"
#include "Menu.h"
#include "Model.h"
#include "MPQ.h"
#include "Project.h"    // This singleton holds later all settings for the current project. Will also be serialized to a selectable place on disk.
#include "revision.h"
#include "Settings.h"    // In this singleton you can insert user settings. This object will later be serialized to disk (userpath)
#include "Video.h"
//#include "shaders.h"

std::vector<AppState*> gStates;
bool gPop = false;

extern std::list<std::string> gListfile;

float gFPS;

freetype::font_data *arialn13, *arial12, *arial14, *arial16, *arial24, *arial32, *morpheus40, *skurri32, *fritz16;  

AsyncLoader* gAsyncLoader;

std::string getGamePath()
{
  if( !FileExists( "NoggIt.conf" ) )
  {
  #ifdef _WIN32
    HKEY key;
    DWORD t;
    const DWORD s = 1024;
    char temp[s];
    memset(temp,0,s);
    LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\Beta",0,KEY_QUERY_VALUE,&key);
    if (l != ERROR_SUCCESS)
      l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\PTR",0,KEY_QUERY_VALUE,&key);
    if (l != ERROR_SUCCESS)
      l = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Blizzard Entertainment\\World of Warcraft",0,KEY_QUERY_VALUE,&key);
    if (l == ERROR_SUCCESS && RegQueryValueEx(key,"InstallPath",0,&t,(LPBYTE)temp,(LPDWORD)&s) == ERROR_SUCCESS) 
      return temp;
    else
      return "";
    RegCloseKey(key);
  #else
    return "/Applications/World of Warcraft/";
  #endif
  }
  else
  {
    Log << "Using config file." << std::endl;
    return ConfigFile( "NoggIt.conf" ).read<std::string>( "Path" );
  }
}

void CreateStrips();

void setApplicationDirectory( const std::string& argv_0 )
{
  std::string fullpath = "";
  if( argv_0.at( 0 ) == '/' || ( argv_0.at( 1 ) == ':' && argv_0.at( 2 ) == '/' ) )
  {
    fullpath = argv_0;
  }
  else
  {
#ifdef _WIN32  
    fullpath = std::string( _getcwd( NULL, 0 ) ) + "/" + argv_0;
#else
    fullpath = std::string( getcwd( NULL, 0 ) ) + "/" + argv_0;
#endif 

  }
  
  fullpath = fullpath.substr( 0, fullpath.find_last_of("/\\") + 1 );
  size_t found = fullpath.find( "/./" );
  while( found != std::string::npos )
  {
    fullpath.replace( found, 3, "/" );
    found = fullpath.find( "/./" );
  }
  found = fullpath.find( "/../" );
  while( found != std::string::npos )
  {
    size_t pos_prev = fullpath.rfind( '/', found - 1 );
    fullpath.replace( pos_prev, found - pos_prev + 4, "/" );
    found = fullpath.find( "/../" );
  }
#ifdef _WIN32  
  _chdir( fullpath.c_str() );
#else
  chdir( fullpath.c_str() );
#endif
}

int main( int argc, char *argv[] )
{
  #ifdef _WIN32
	// hide the console window on windows
	HWND hWnd = GetConsoleWindow();
    ShowWindow( hWnd, SW_HIDE );
  #endif
  RegisterErrorHandlers();
  setApplicationDirectory( argv[0] );

  // Set up log.
  InitLogging();

  Settings::getInstance();
  Project::getInstance();
  Environment::getInstance();

  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;
  
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
    LogError << "Can not find arial.ttf. This is really weird if you have windows. Add the file to the noggit directory then!" << std::endl;
    return -1;
  }
  
  srand( time( NULL ) );
  
  int xres = 1280;
  int yres = 720;
  bool fullscreen = false;
  
  // handle starting parameters
  for( int i = 1; i < argc; ++i ) 
  {
    if( !strcmp( argv[i], "-f" ) || !strcmp( argv[i], "-fullscreen" ) ) 
      fullscreen = true;
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
    else if (!strcmp(argv[i],"-1280x720")) {
      xres = 1280;
      yres = 720;
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
  
  SDL_WM_SetCaption( "Noggit Studio - " STRPRODUCTVER, "" );
  
  std::string wowpath = getGamePath();
  if( wowpath == "" )
  {
    return -1;
  }
  
  Log << "Game path: " << wowpath << std::endl;
  
  if( Project::getInstance()->getPath() == "" )
    Project::getInstance()->setPath( wowpath );
  Log << "Project path: " << Project::getInstance()->getPath() << std::endl;
  
  CreateStrips();
  
  gAsyncLoader->start(1); 

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
  for( int i = 0; i < 10; ++i )
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
  for( size_t i = 0; i < archiveNames.size(); ++i )
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
	  std::stringstream temp;
      location = path.find( "{number}" );
      path.replace( location, 8, " " );
      for( int j = 2; j < 10; j++ )
      {
		temp << j;
        path.replace( location, 1, temp.str() );
        if( FileExists( path ) )
          gAsyncLoader->addObject( new MPQArchive( path, true ) );
      }
    }
    else if( path.find( "{character}" ) != std::string::npos  )
    {
	  std::stringstream temp;
      location = path.find( "{character}" );
      path.replace( location, 11, " " );
      for( char c = 'a'; c <= 'z'; c++ )
      {
		temp << c;
        path.replace( location, 1, temp.str() );
        if( FileExists( path ) )
          gAsyncLoader->addObject( new MPQArchive( path, true ) );
      }
    }
    else
      if( FileExists( path ) )
        gAsyncLoader->addObject( new MPQArchive( path, true ) );
  }
  
  // listfiles are not available straight away! They are async! Do not rely on anything at this point!
  
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
  /*  std::list<std::string>::iterator it;
   for( it = gListfile.begin(); it != gListfile.end(); ++it )
   {
   found = it->find_last_of("/\\");
   if( found != std::string::npos )
   gFileList->AddSubDirectory( it->substr(0,found) )->AddFile( it->substr(found+1) );
   else
   gFileList->AddFile( *it );
   }
   */
  
  // Opening DBCs
  OpenDBs();
  
  // Initializing Fonts
  skurri32 = new freetype::font_data( "fonts\\SKURRI.TTF", 32, true );
  fritz16 = new freetype::font_data( "fonts\\FRIZQT__.TTF", 16, true );
  morpheus40 = new freetype::font_data( "fonts\\MORPHEUS.TTF", 40, true );
  arialn13 = new freetype::font_data( "fonts\\arialn.TTF", 13, true );
  
  const char* arialname = lFontWindows ? "C:\\windows\\fonts\\arial.ttf" : "fonts/arial.ttf";
  arial12 = new freetype::font_data( arialname, 12, false );
  arial14 = new freetype::font_data( arialname, 14, false );
  arial16 = new freetype::font_data( arialname, 16, false );
  arial24 = new freetype::font_data( arialname, 24, false );
  arial32 = new freetype::font_data( arialname, 32, false );
  
  float ftime;
  Uint32 t, last_t, frames = 0, time = 0, fcount = 0, ft = 0;
  AppState *as;
  gFPS = 0;
  
  LogDebug << "Creating Menu" << std::endl;
  
  Menu *m = new Menu();
  as = m;


  gStates.push_back( as );
  
  if( video.mSupportShaders )
    loadWaterShader();
  else
    LogError << "Your GPU does not support ARB vertex programs (shaders). Sorry." << std::endl;
  
  bool done = false;
  t = SDL_GetTicks();
  
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
        // message to the active gui element
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
      as = NULL;
    }
    
    frames++;
    fcount++;
    ft += dt;
    if (ft >= 1000) 
    {
      gFPS = static_cast<float>(fcount) / static_cast<float>(ft) * 1000.0f;
      ft = 0;
      fcount = 0;
    }
    
    video.flip();
  }
  
  video.close();
  
  gAsyncLoader->stop();
  gAsyncLoader->join();
  
  for(std::vector<MPQArchive*>::iterator it=gOpenArchives.begin(); it!=gOpenArchives.end();++it)
  {
    delete *it;
  }
  gOpenArchives.clear();
  
  delete arialn13;
  delete arial12;
  delete arial14;
  delete arial16;
  delete arial24;
  delete arial32;
  delete morpheus40;
  delete skurri32;
  delete fritz16;  
  
  LogDebug << "Exited" << std::endl;
  
  return 0;
}
