// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>
#include <noggit/MapView.h>
#include <noggit/Model.h>
#include <noggit/ModelManager.h> // ModelManager::report()
#include <noggit/TextureManager.h> // TextureManager::report()
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
#include <string>
#include <vector>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QOffscreenSurface>
#include <QtOpenGL/QGLFormat>
#include <QtCore/QDir>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>

#include "revision.h"

class Noggit
{
public:
  Noggit (int argc, char *argv[]);

private:
  void initPath(char *argv[]);
  void parseArgs(int argc, char *argv[]);
  void loadMPQs();

  std::unique_ptr<noggit::ui::main_window> main_window;

  boost::filesystem::path wowpath;

  std::unique_ptr<AsyncLoader> asyncLoader;

  bool fullscreen;
  bool doAntiAliasing;
};

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
  }
  QSettings settings;
  if (!settings.value("antialiasing", true).toBool())
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
          MPQArchive::loadMPQ (asyncLoader.get(), path, true);
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
          MPQArchive::loadMPQ (asyncLoader.get(), path, true);
      }
    }
    else
      if (boost::filesystem::exists(path))
        MPQArchive::loadMPQ (asyncLoader.get(), path, true);
  }
}

bool is_valid_game_path (const QDir& path)
{
  if (!path.exists ())
  {
    LogError << "Path \"" << qPrintable (path.absolutePath ())
      << "\" does not exist." << std::endl;
    return false;
  }

  QStringList locales;
  locales << "enGB" << "enUS" << "deDE" << "koKR" << "frFR"
    << "zhCN" << "zhTW" << "esES" << "esMX" << "ruRU";
  QString found_locale ("****");

  foreach (const QString& locale, locales)
  {
    if (path.exists (("Data/" + locale)))
    {
      found_locale = locale;
      break;
    }
  }

  if (found_locale == "****")
  {
    LogError << "Path \"" << qPrintable (path.absolutePath ())
      << "\" does not contain a locale directory "
      << "(invalid installation or no installation at all)."
      << std::endl;
    return false;
  }

  return true;
}

Noggit::Noggit(int argc, char *argv[])
  : fullscreen(false)
  , doAntiAliasing(true)
{
  InitLogging();
  initPath(argv);

  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;

  parseArgs(argc, argv);

  srand(::time(nullptr));
  QSettings settings;
  QDir path = settings.value ("project/game_path").toString();

  while (!is_valid_game_path (path))
  {
    path = QFileDialog::getExistingDirectory (nullptr, "Open WoW Directory", "/", QFileDialog::ShowDirsOnly);
    if (path.absolutePath () == "")
    {
      LogError << "Could not auto-detect game path "
        << "and user canceled the dialog." << std::endl;
      throw std::runtime_error ("no folder chosen");
    }
  }

  wowpath = path.absolutePath().toStdString();

  Log << "Game path: " << wowpath << std::endl;

  std::string project_path = settings.value ("project/path", path.absolutePath()).toString().toStdString();
  settings.setValue ("project/path", QString::fromStdString (project_path));

  Log << "Project path: " << project_path << std::endl;

  settings.setValue ("project/game_path", path.absolutePath());
  settings.setValue ("project/path", QString::fromStdString(project_path));

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
  format.setSwapInterval(settings.value ("vsync", 0).toInt());

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
  if (fullscreen)
  {
    main_window->showFullScreen();
  }
  else
  {
    main_window->showMaximized();
  }  
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
  noggit::RegisterErrorHandlers();

  QApplication qapp (argc, argv);
  qapp.setApplicationName ("Noggit");
  qapp.setOrganizationName ("Noggit");

  Noggit app (argc, argv);

  return qapp.exec();
}
