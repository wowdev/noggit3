#include "Noggit.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <winerror.h>
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#include "SDL_syswm.h"
HINSTANCE hInst;
char* gpszProgramName = "Noggit3";
static LOGCONTEXT	glogContext = { 0 };
#endif

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#include <SDL.h>

#include "AppState.h"
#include "AsyncLoader.h"
#include "ConfigFile.h"
#include "Environment.h"  // This singleton holds all vars you dont must save. Like bools for display options. We should move all global stuff here to get it OOP!
#include "errorHandling.h"
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
#include "TextureManager.h" // TextureManager::report()
#include "WMO.h" // WMOManager::report()
#include "ModelManager.h" // ModelManager::report()

Noggit app;
void CreateStrips();
extern std::list<std::string> gListfile;

Noggit::Noggit()
	: fullscreen(false)
	, doAntiAliasing(true)
	, xres(1280)
	, yres(720)
{}

Noggit::~Noggit()
{

}

void Noggit::initPath(char *argv[])
{
	try
	{
		boost::filesystem::path startupPath(argv[0]);
		startupPath.remove_filename();

		if (startupPath.is_relative())
		{
			boost::filesystem::current_path(boost::filesystem::current_path() / startupPath);
		}
		else
		{
			boost::filesystem::current_path(startupPath);
		}
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		LogError << ex.what() << std::endl;
	}
}

void Noggit::initFont()
{

	std::string arialFilename("<PLEASE GET SOME FONT FOR YOUR OS>");
#ifdef _WIN32
	//! \todo This might not work on windows 7 or something. Please fix.
	arialFilename = "C:\\windows\\fonts\\arial.ttf";
#endif
#ifdef __APPLE__
	arialFilename = "/Library/Fonts/Arial.ttf";
#endif
	if (!boost::filesystem::exists(arialFilename))
	{
		arialFilename = "arial.ttf";
		if (!boost::filesystem::exists(arialFilename))
		{
			arialFilename = "fonts/arial.ttf";
			if (!boost::filesystem::exists(arialFilename))
			{
				LogError << "Can not find arial.ttf." << std::endl;
				//return -1;
			}
		}
	}

	// Initializing Fonts
	skurri32.init("fonts/skurri.ttf", 32, true);
	fritz16.init("fonts/frizqt__.ttf", 16, true);
	morpheus40.init("fonts/morpheus.ttf", 40, true);
	arialn13.init("fonts/arialn.ttf", 13, true);

	arial12.init(arialFilename, 12, false);
	arial14.init(arialFilename, 14, false);
	arial16.init(arialFilename, 16, false);
	arial24.init(arialFilename, 24, false);
	arial32.init(arialFilename, 32, false);

}

void Noggit::initEnv()
{
#ifdef _WIN32
	//this is for graphics tablet (e.g. Wacom, Huion, possibly others) initialization.
	SDL_SysWMinfo SysInfo;
	SDL_GetWMInfo(&SysInfo);
	WindowHandle = SysInfo.window;
	hInst = (HINSTANCE)GetWindowLong(WindowHandle, GWL_HINSTANCE);
	hCtx = NULL;
	tabletActive = FALSE;

	if (LoadWintab())
	{
		/* check if WinTab available. */
		if (gpWTInfoA(0, 0, NULL))
		{
			hCtx = TabletInit(WindowHandle);
			gpWTEnable(hCtx, TRUE);
			gpWTOverlap(hCtx, TRUE);
			if (!hCtx)
			{
				Log << "Could Not Open Tablet Context." << std::endl;
			}
			else
			{
				Log << "Opened Tablet Context." << std::endl;
			}
			tabletActive = TRUE;
		}
	}
#endif

	// init
	Environment::getInstance()->cursorColorR = 1.0f;
	Environment::getInstance()->cursorColorG = 1.0f;
	Environment::getInstance()->cursorColorB = 1.0f;
	Environment::getInstance()->cursorColorA = 1.0f;
	Environment::getInstance()->cursorType = 1;

	// load cursor settings
	if (boost::filesystem::exists("noggit.conf"))
	{
		ConfigFile myConfigfile = ConfigFile("noggit.conf");
		if (myConfigfile.keyExists("RedColor") && myConfigfile.keyExists("GreenColor") && myConfigfile.keyExists("BlueColor") && myConfigfile.keyExists("AlphaColor"))
		{
			Environment::getInstance()->cursorColorR = myConfigfile.read<float>("RedColor");
			Environment::getInstance()->cursorColorG = myConfigfile.read<float>("GreenColor");
			Environment::getInstance()->cursorColorB = myConfigfile.read<float>("BlueColor");
			Environment::getInstance()->cursorColorA = myConfigfile.read<float>("AlphaColor");
		}

		if (myConfigfile.keyExists("CursorType"))
			Environment::getInstance()->cursorType = myConfigfile.read<int>("CursorType");
	}

	Settings::getInstance();
	Project::getInstance();
	Environment::getInstance();
}

void Noggit::parseArgs(int argc, char *argv[])
{
	// handle starting parameters
	for (int i(1); i < argc; ++i)
	{
		if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "-fullscreen"))
		{
			fullscreen = true;
		}
		else if (!strcmp(argv[i], "-na") || !strcmp(argv[i], "-noAntiAliasing"))
		{
			doAntiAliasing = false;
		}
		else if (!strcmp(argv[i], "-1024") || !strcmp(argv[i], "-1024x768")) {
			xres = 1024;
			yres = 768;
		}
		else if (!strcmp(argv[i], "-800") || !strcmp(argv[i], "-800x600")) {
			xres = 800;
			yres = 600;
		}
		else if (!strcmp(argv[i], "-1280") || !strcmp(argv[i], "-1280x1024")) {
			xres = 1280;
			yres = 1024;
		}
		else if (!strcmp(argv[i], "-1280x960")) {
			xres = 1280;
			yres = 960;
		}
		else if (!strcmp(argv[i], "-1280x720")) {
			xres = 1280;
			yres = 720;
		}
		else if (!strcmp(argv[i], "-1400") || !strcmp(argv[i], "-1400x1050")) {
			xres = 1400;
			yres = 1050;
		}
		else if (!strcmp(argv[i], "-1280x800")) {
			xres = 1280;
			yres = 800;
		}
		else if (!strcmp(argv[i], "-1600") || !strcmp(argv[i], "-1600x1200")) {
			xres = 1600;
			yres = 1200;
		}
		else if (!strcmp(argv[i], "-1920") || !strcmp(argv[i], "-1920x1200")) {
			xres = 1920;
			yres = 1200;
		}
		else if (!strcmp(argv[i], "-1080p") ){
			xres = 1920;
			yres = 1080;
		}
		else if (!strcmp(argv[i], "-2048") || !strcmp(argv[i], "-2048x1536")) {
			xres = 2048;
			yres = 1536;
		}
	}

	if (Settings::getInstance()->noAntiAliasing())
	{
		doAntiAliasing = false;
	}
}

std::string Noggit::getGamePath()
{
	if (!boost::filesystem::exists("noggit.conf"))
	{
		Log << "DON NOT find a config file." << std::endl;

		if (boost::filesystem::exists("noggit.conf.conf"))
		{
			Log << "Error: You have named your config file noggit.conf.conf!" << std::endl;
			Log << "Erase the second .conf!" << std::endl;
		}
		else if (boost::filesystem::exists("noggit_template.conf"))
		{
			Log << "You must rename noggit_template.conf to noggit.conf if noggit should use the config file!" << std::endl;
		}


#ifdef _WIN32
		Log << "Will try to load the game path from you registry now:" << std::endl;
		HKEY key;
		DWORD t;
		const DWORD s(1024);
		char temp[s];
		memset(temp, 0, s);
		LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Blizzard Entertainment\\World of Warcraft", 0, KEY_QUERY_VALUE, &key);
		if (l != ERROR_SUCCESS)
			l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\PTR", 0, KEY_QUERY_VALUE, &key);
		if (l != ERROR_SUCCESS)
			l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Blizzard Entertainment\\World of Warcraft", 0, KEY_QUERY_VALUE, &key);
		if (l == ERROR_SUCCESS && RegQueryValueEx(key, "InstallPath", 0, &t, (LPBYTE)temp, (LPDWORD)&s) == ERROR_SUCCESS)
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
		return ConfigFile("noggit.conf").read<std::string>("Path");
	}
}


void Noggit::loadMPQs()
{
	asyncLoader = new AsyncLoader();
	asyncLoader->start(1);

	std::vector<std::string> archiveNames;
	archiveNames.push_back("common.MPQ");
	archiveNames.push_back("common-2.MPQ");
	archiveNames.push_back("expansion.MPQ");
	archiveNames.push_back("lichking.MPQ");
	archiveNames.push_back("patch.MPQ");
	archiveNames.push_back("patch-{number}.MPQ");
	archiveNames.push_back("patch-{character}.MPQ");

	//archiveNames.push_back( "{locale}/backup-{locale}.MPQ" );
	//archiveNames.push_back( "{locale}/base-{locale}.MPQ" );
	archiveNames.push_back("{locale}/locale-{locale}.MPQ");
	//archiveNames.push_back( "{locale}/speech-{locale}.MPQ" );
	archiveNames.push_back("{locale}/expansion-locale-{locale}.MPQ");
	//archiveNames.push_back( "{locale}/expansion-speech-{locale}.MPQ" );
	archiveNames.push_back("{locale}/lichking-locale-{locale}.MPQ");
	//archiveNames.push_back( "{locale}/lichking-speech-{locale}.MPQ" );
	archiveNames.push_back("{locale}/patch-{locale}.MPQ");
	archiveNames.push_back("{locale}/patch-{locale}-{number}.MPQ");
	archiveNames.push_back("{locale}/patch-{locale}-{character}.MPQ");

	archiveNames.push_back("development.MPQ");

	const char * locales[] = { "enGB", "enUS", "deDE", "koKR", "frFR", "zhCN", "zhTW", "esES", "esMX", "ruRU" };
	const char * locale("****");

	// Find locale, take first one.
	for (int i(0); i < 10; ++i)
	{
		std::string path(wowpath);
		path.append("Data/").append(locales[i]).append("/realmlist.wtf");
		if (boost::filesystem::exists(path))
		{
			locale = locales[i];
			Log << "Locale: " << locale << std::endl;
			break;
		}
	}
	if (!strcmp(locale, "****"))
	{
		LogError << "Could not find locale directory. Be sure, that there is one containing the file \"realmlist.wtf\"." << std::endl;
		//return -1;
	}

	//! \todo  This may be done faster. Maybe.
	for (size_t i(0); i < archiveNames.size(); ++i)
	{
		std::string path(wowpath);
		path.append("Data/").append(archiveNames[i]);
		std::string::size_type location(std::string::npos);

		do
		{
			location = path.find("{locale}");
			if (location != std::string::npos)
			{
				path.replace(location, 8, locale);
			}
		} while (location != std::string::npos);

		if (path.find("{number}") != std::string::npos)
		{
			location = path.find("{number}");
			path.replace(location, 8, " ");
			for (char j = '2'; j <= '9'; j++)
			{
				path.replace(location, 1, std::string(&j, 1));
				if (boost::filesystem::exists(path))
					MPQArchive::loadMPQ(path, true);
			}
		}
		else if (path.find("{character}") != std::string::npos)
		{
			location = path.find("{character}");
			path.replace(location, 11, " ");
			for (char c = 'a'; c <= 'z'; c++)
			{
				path.replace(location, 1, std::string(&c, 1));
				if (boost::filesystem::exists(path))
					MPQArchive::loadMPQ(path, true);
			}
		}
		else
			if (boost::filesystem::exists(path))
				MPQArchive::loadMPQ(path, true);
	}
}

void Noggit::mainLoop()
{
	uint32_t timeA, timeB, diff;
	bool done(false);
	Uint32 ticks(SDL_GetTicks());
	AppState* activeAppState(NULL);
	Uint32 time(0);

	SDL_EnableUNICODE(true);

	SDL_Event event;
	 
	while (!states.empty() && !done)
	{
		timeA = SDL_GetTicks();

		Uint32 lastTicks(ticks);
		ticks = SDL_GetTicks();
		Uint32 tickDelta(ticks - lastTicks);
		time += tickDelta;

		activeAppState = states[states.size() - 1];

		const Uint8 appState(SDL_GetAppState());
		const bool isActiveApplication((appState & SDL_APPACTIVE) != 0);
		const bool hasInputFocus((appState & SDL_APPINPUTFOCUS) != 0);
		const bool hasMouseFocus(appState & SDL_APPMOUSEFOCUS);

		if (isActiveApplication)
		{
			const float ftime(time / 1000.0f);
			const float ftickDelta(tickDelta / 1000.0f);
			activeAppState->tick(ftime, ftickDelta);
			activeAppState->display(ftime, ftickDelta);
			video.flip();
		}
		else
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(200));
		}

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
        SDL_KeyboardEvent e;
        e.type = SDL_KEYDOWN;
        e.keysym.sym = SDLK_ESCAPE;
        e.keysym.mod = KMOD_NONE;
        activeAppState->keypressed(&e);
			}
			else if (event.type == SDL_VIDEORESIZE)
			{
				video.resize(event.resize.w, event.resize.h);
				activeAppState->resizewindow();
			}
			else if (hasInputFocus)
			{
				if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP))
				{
					activeAppState->keypressed(&event.key);
				}
				else if (hasMouseFocus)
				{
					if (event.type == SDL_MOUSEMOTION)
					{
						activeAppState->mousemove(&event.motion);
					}
					else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
					{
						activeAppState->mouseclick(&event.button);
					}
				}
			}
		}
		if (pop)
		{
			pop = false;
			states.pop_back();
			delete activeAppState;
			activeAppState = NULL;
		}
#ifdef _WIN32
		if (tabletActive)
		{
      PACKET pkt;
			while (gpWTPacketsGet(hCtx, 1, &pkt) > 0) //this is a while because we really only want the last packet.
			{
				pressure = pkt.pkNormalPressure;
			}
		}
#endif
		timeB = SDL_GetTicks();
		diff = timeB - timeA;
		if (diff > 0)
			FPS = (float)(1000.0f / (float)diff);
	}
}

int Noggit::start(int argc, char *argv[])
{
	InitLogging();
	initPath(argv);

	Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;

	initEnv();
	parseArgs(argc, argv);
	srand((unsigned int)time(NULL));
	wowpath = getGamePath();

	if (wowpath == "")
		return -1;
	Log << "Game path: " << wowpath << std::endl;

	if (Project::getInstance()->getPath() == "")
		Project::getInstance()->setPath(wowpath);
	Log << "Project path: " << Project::getInstance()->getPath() << std::endl;

	CreateStrips();
	loadMPQs(); // listfiles are not available straight away! They are async! Do not rely on anything at this point!
	OpenDBs();

	if (!video.init(xres, yres, fullscreen, doAntiAliasing))
	{
		LogError << "Initializing video failed." << std::endl;
		return -1;
	}

	SDL_WM_SetCaption("Noggit Studio - " STRPRODUCTVER, "");
	initFont();

	if (video.mSupportShaders)
		loadWaterShader();
	else
		LogError << "Your GPU does not support ARB vertex programs (shaders). Sorry." << std::endl;

	LogDebug << "Creating Menu" << std::endl;
	states.push_back(new Menu());

	LogDebug << "Entering Main Loop" << std::endl;
	mainLoop();

	video.close();

	TextureManager::report();
	ModelManager::report();
	WMOManager::report();

	asyncLoader->stop();
	asyncLoader->join();

	MPQArchive::unloadAllMPQs();
	gListfile.clear();

	LogDebug << "Exited" << std::endl;


	return 0;
}

#ifdef _WIN32
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[])
{
	RegisterErrorHandlers();
	return app.start(argc, argv);
}

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
#ifdef _WIN32
HCTX static NEAR TabletInit(HWND hWnd)
{
	HCTX hctx = NULL;
	UINT wDevice = 0;
	UINT wExtX = 0;
	UINT wExtY = 0;
	UINT wWTInfoRetVal = 0;
	AXIS TabletX = { 0 };
	AXIS TabletY = { 0 };

	// Set option to move system cursor before getting default system context.
	glogContext.lcOptions |= CXO_SYSTEM;

	// Open default system context so that we can get tablet data
	// in screen coordinates (not tablet coordinates).
	wWTInfoRetVal = gpWTInfoA(WTI_DEFSYSCTX, 0, &glogContext);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(LOGCONTEXT));

	WACOM_ASSERT(glogContext.lcOptions & CXO_SYSTEM);

	// modify the digitizing region
	wsprintf(glogContext.lcName, "PrsTest Digitizing %x", hInst);

	// We process WT_PACKET (CXO_MESSAGES) messages.
	glogContext.lcOptions |= CXO_MESSAGES;

	// What data items we want to be included in the tablet packets
	glogContext.lcPktData = PACKETDATA;

	// Which packet items should show change in value since the last
	// packet (referred to as 'relative' data) and which items
	// should be 'absolute'.
	glogContext.lcPktMode = PACKETMODE;

	// This bitfield determines whether or not this context will receive
	// a packet when a value for each packet field changes.  This is not
	// supported by the Intuos Wintab.  Your context will always receive
	// packets, even if there has been no change in the data.
	glogContext.lcMoveMask = PACKETDATA;

	// Which buttons events will be handled by this context.  lcBtnMask
	// is a bitfield with one bit per button.
	glogContext.lcBtnUpMask = glogContext.lcBtnDnMask;

	// Set the entire tablet as active
	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_Y, &TabletY);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	glogContext.lcInOrgX = 0;
	glogContext.lcInOrgY = 0;
	glogContext.lcInExtX = TabletX.axMax;
	glogContext.lcInExtY = TabletY.axMax;

	// Guarantee the output coordinate space to be in screen coordinates.
	glogContext.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	glogContext.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	glogContext.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN); //SM_CXSCREEN );

																  // In Wintab, the tablet origin is lower left.  Move origin to upper left
																  // so that it coincides with screen origin.
	glogContext.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);	//SM_CYSCREEN );

																	// Leave the system origin and extents as received:
																	// lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

																	// open the region
																	// The Wintab spec says we must open the context disabled if we are
																	// using cursor masks.
	hctx = gpWTOpenA(hWnd, &glogContext, FALSE);

	WacomTrace("HCTX: %i\n", hctx);

	return hctx;
}
#endif
