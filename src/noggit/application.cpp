// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/application.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <winerror.h>
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
HINSTANCE hInst;
char* gpszProgramName = "Noggit3";
static LOGCONTEXT  glogContext = { 0 };
#endif

#include <noggit/Native.hpp>

#include <noggit/ConfigFile.h>
#include <noggit/Environment.h>  // This singleton holds all vars you dont must save. Like bools for display options. We should move all global stuff here to get it OOP!
#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <noggit/MapView.h>
#include <noggit/Model.h>
#include <noggit/ModelManager.h> // ModelManager::report()
#include <noggit/Project.h>    // This singleton holds later all settings for the current project. Will also be serialized to a selectable place on disk.
#include <noggit/Settings.h>    // In this singleton you can insert user settings. This object will later be serialized to disk (userpath)
#include <noggit/TextureManager.h> // TextureManager::report()
#include <noggit/Video.h>
#include <noggit/ui/font_awesome.hpp>
#include <noggit/WMO.h> // WMOManager::report()
#include <noggit/errorHandling.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/ui/main_window.hpp>
#include <opengl/context.hpp>

#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>

#include <QtCore/QTimer>
#include <QtGui/QOffscreenSurface>
#include <QtOpenGL/QGLFormat>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>

#include <QtWidgets/QOpenGLWidget>

#include "revision.h"

Noggit app;

Noggit::Noggit()
  : fullscreen(false)
  , doAntiAliasing(true)
  , xres(1280)
  , yres(720)
{}

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
  std::string arialFilename = Native::getArialPath();

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

  noggit::ui::font_awesome::load();
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

void Noggit::loadMPQs()
{
  asyncLoader = std::make_unique<AsyncLoader>();
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
    if (boost::filesystem::exists (wowpath / "Data" / locales[i] / "realmlist.wtf"))
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
    std::string path((wowpath / "Data" / archiveNames[i]).string());
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

int Noggit::start(int argc, char *argv[])
{
  InitLogging();
  initPath(argv);

  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;

  parseArgs(argc, argv);

  srand(::time(nullptr));
  wowpath = Settings::getInstance()->gamePath;

  if (wowpath == "")
  {
    LogError << "Empty wow path" << std::endl;
    return -1;
  }

  boost::filesystem::path data_path = wowpath / "Data";

  if (!boost::filesystem::exists(data_path))
  {
    LogError << "Could not find data directory: " << data_path << std::endl;
    return -1;
  }

  Log << "Game path: " << wowpath << std::endl;

  if (Project::getInstance()->getPath() == "")
    Project::getInstance()->setPath(wowpath.string());
  Log << "Project path: " << Project::getInstance()->getPath() << std::endl;

  loadMPQs(); // listfiles are not available straight away! They are async! Do not rely on anything at this point!
  OpenDBs();

  if (!QGLFormat::hasOpenGL())
  {
    throw std::runtime_error ("Your system does not support OpenGL. Sorry, this application can't run without it.");
  }

  QSurfaceFormat format;

  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setVersion(2, 1);

  format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  format.setSwapInterval(1);

  if (doAntiAliasing)
  {
    format.setSamples (4);
  }

  QSurfaceFormat::setDefaultFormat (format);

  QOpenGLContext context;
  context.create();
  QOffscreenSurface surface;
  surface.create();
  context.makeCurrent (&surface);

  opengl::context::scoped_setter const _ (::gl, &context);

  LogDebug << "GL: Version: " << gl.getString (GL_VERSION) << std::endl;
  LogDebug << "GL: Vendor: " << gl.getString (GL_VENDOR) << std::endl;
  LogDebug << "GL: Renderer: " << gl.getString (GL_RENDERER) << std::endl;

  main_window = std::make_unique<noggit::ui::main_window>();
  main_window->resize (xres, yres);
  main_window->show();

  return 0;
}


#ifdef _WIN32
int main(int argc, char *argv[]);
int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[])
{
  RegisterErrorHandlers();

  QApplication qapp (argc, argv);

  if (int res = app.start (argc, argv))
  {
    return res;
  }

  return qapp.exec();
}
