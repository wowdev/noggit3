#include "Noggit.h"

#include <GL/glew.h>

#include <QDir>
#include <QSettings>
#include <QTime>
#include <QGLWidget>
#include <QGLFormat>
#include <QFontDatabase>

#include "revision.h"

#include "AsyncLoader.h"
#include "DBC.h"
#include "errorHandling.h"
#include "Menu.h"
#include "MapView.h"
#include "TextureManager.h" // TextureManager::report()
#include "WMO.h" // WMOManager::report()
#include "ModelManager.h" // ModelManager::report()

//! \todo Remove.
#include "FreeType.h" // fonts.
#include "Environment.h"
#include "ConfigFile.h"

AsyncLoader* gAsyncLoader;

freetype::font_data arialn13;
freetype::font_data arial12;
freetype::font_data arial14;
freetype::font_data arial16;
freetype::font_data arial24;
freetype::font_data arial32;
freetype::font_data morpheus40;
freetype::font_data skurri32;
freetype::font_data fritz16;

void CreateStrips();

Noggit::Noggit (int& argc, char** argv)
  : QApplication (argc, argv)
  , _settings (NULL)
  , _async_loader (new AsyncLoader())
{
  RegisterErrorHandlers();
  InitLogging();
  Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;

  setOrganizationDomain ("modcraft.tk");
  setOrganizationName ("Modcraft");
  setApplicationName ("Noggit");
  _settings = new QSettings (this);

  qsrand (QTime::currentTime().msec());

  _async_loader->start( 1 );
  //! \todo remove
  gAsyncLoader = _async_loader;

  set_working_directory_to_application_path();
  parse_command_line_and_set_defaults();
  get_game_path();
  open_mpqs();

  add_font_from_mpq ("fonts/skurri.ttf");
  add_font_from_mpq ("fonts/frizqt__.ttf");
  add_font_from_mpq ("fonts/morpheus.ttf");
  add_font_from_mpq ("fonts/arialn.ttf");

  CreateStrips();
  OpenDBs();

  initialize_video();

  Menu* map_selection_menu (new Menu (NULL));
  connect (map_selection_menu, SIGNAL (create_world_view_request (World*)), SLOT (create_world_view (World*)));
  map_selection_menu->show();
}

Noggit::~Noggit()
{
  _async_loader->stop();
  _async_loader->join();

  TextureManager::report();
  ModelManager::report();
  WMOManager::report();

  MPQArchive::unloadAllMPQs();
}

void Noggit::set_working_directory_to_application_path()
{
  QString appPath (applicationDirPath());
  static const QString appInternal ("/Contents/MacOS");
  if (appPath.endsWith (appInternal))
  {
    appPath.remove (appPath.lastIndexOf (appInternal), appInternal.size());
    appPath = appPath.left (appPath.lastIndexOf ('/'));
  }
  QDir::setCurrent (appPath);
}

void Noggit::parse_command_line_and_set_defaults()
{
  int xResolution (_settings->value ("resolution/x", 1024).toInt());
  int yResolution (_settings->value ("resolution/y", 768).toInt());
  bool inFullscreen (_settings->value ("fullscreen", false).toBool());
  bool doAntiAliasing (_settings->value ("antialiasing", true).toBool());
  int view_distance (_settings->value ("view_distance", 2048).toInt());

  foreach (const QString& argument, arguments())
  {
    if (argument == "-f" || argument == "--fullscreen")
    {
      inFullscreen = true;
    }
    else if (argument == "-noaa" || argument == "--noantialiasing")
    {
      doAntiAliasing = false;
    }
    else
    {
      QRegExp resolution ("-(-resolution=)?(\\d+)(x(\\d+))*");

      if (!resolution.indexIn (argument))
      {
        const QStringList matched (resolution.capturedTexts());
        xResolution = matched.at (2).toInt();
        yResolution = matched.at (4).size() > 0
                    ? matched.at (4).toInt()
                    : xResolution * 0.75;
      }
    }
    //! \todo View distance.
  }

  _settings->setValue ("resolution/x", xResolution);
  _settings->setValue ("resolution/y", yResolution);
  _settings->setValue ("fullscreen", inFullscreen);
  _settings->setValue ("antialiasing", doAntiAliasing);
  _settings->setValue ("view_distance", view_distance);
  _settings->sync();
}

class dummy_gl_widget : public QGLWidget
{
public:
  dummy_gl_widget (const QGLFormat& format)
    : QGLWidget (format)
  {
    updateGL();
  }
protected:
  virtual void initializeGL()
  {
    const GLenum err (glewInit());
    if( GLEW_OK != err )
    {
      LogError << "GLEW: " << glewGetErrorString (err) << std::endl;
      throw std::runtime_error ("unable to initialize glew.");
    }

    LogDebug << "GL: Version: " << glGetString (GL_VERSION) << std::endl;
    LogDebug << "GL: Vendor: " << glGetString (GL_VENDOR) << std::endl;
    LogDebug << "GL: Renderer: " << glGetString (GL_RENDERER) << std::endl;
  }
};

void Noggit::initialize_video()
{
  if (!QGLFormat::hasOpenGL())
  {
    LogError << "Your system does not support OpenGL. Sorry, this application can't run without it." << std::endl;
  }

  QGLFormat format;
  format.setStencilBufferSize (1);
  format.setDepthBufferSize (16);
  format.setAlphaBufferSize (8);

  if (false && _settings->value ("antialiasing").toBool())
  {
    format.setSampleBuffers (true);
    format.setSamples (4);
  }

  _dummy_gl_widget = new dummy_gl_widget (format);

  video.farclip (_settings->value ("view_distance").toInt());
}

void Noggit::get_game_path()
{
  QVariant game_path_variant (_settings->value ("paths/game"));

  if (game_path_variant.isValid())
  {
    _game_path = game_path_variant.toString();
  }
  else
  {
#ifdef Q_WS_WIN
    static const QString default_registry_path
      ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Blizzard Entertainment\\World of Warcraft");

    QSettings registry (default_registry_path, QSettings::NativeFormat | QSettings::Registry32Format);

    _game_path = registry.value ("InstallPath").toString();
#else
#ifdef Q_WS_MAC
    _game_path = "/Applications/World of Warcraft/";
#else
    _game_path = "NO DEFAULT PATH FOR YOUR OS";
#endif
#endif
  }

  if (!QFile::exists (_game_path))
  {
    LogError << "Nonexisting game-path set: " << qPrintable (_game_path) << std::endl;
    throw std::runtime_error ("Nonexisting game-path set.");
  }

  _project_path = _settings->value ("paths/project", _game_path).toString();

  _locale = _settings->value ("locale", "****").toString();

  if (_locale == "****")
  {
    QStringList locales;
    locales << "enGB" << "enUS" << "deDE" << "koKR" << "frFR"
            << "zhCN" << "zhTW" << "esES" << "esMX" << "ruRU";

    foreach (const QString& locale, locales)
    {
      if (QFile::exists (_game_path + "Data/" + locale))
      {
        _locale = locale;
        break;
      }
    }

    if (_locale == "****")
    {
      LogError << "Could not find locale directory. Sorry." << std::endl;
      throw std::runtime_error ("Could not find locale directory. Sorry.");
    }
  }

  _settings->setValue ("paths/game", _game_path);
  _settings->setValue ("paths/project", _project_path);
  _settings->setValue ("locale", _locale);
  _settings->sync();

  Log << "Game path: " << qPrintable (_game_path) << std::endl;
  Log << "Project path: " << qPrintable (_project_path) << std::endl;
}

void Noggit::open_mpqs()
{
  QStringList archive_names;
  archive_names << "common.MPQ"
                << "common-2.MPQ"
                << "expansion.MPQ"
                << "lichking.MPQ"
                << "patch.MPQ"
                << "patch-%1.MPQ"
              //<< "{locale}/backup-{locale}.MPQ"
              //<< "{locale}/base-{locale}.MPQ"
                << "{locale}/locale-{locale}.MPQ"
              //<< "{locale}/speech-{locale}.MPQ"
                << "{locale}/expansion-locale-{locale}.MPQ"
              //<< "{locale}/expansion-speech-{locale}.MPQ"
                << "{locale}/lichking-locale-{locale}.MPQ"
              //<< "{locale}/lichking-speech-{locale}.MPQ"
                << "{locale}/patch-{locale}.MPQ"
                << "{locale}/patch-{locale}-%1.MPQ"
              //<< "development.MPQ"
                 ;

  foreach (const QString& archive, archive_names)
  {
    QString path (_game_path + "Data/" + archive);
    path.replace ("{locale}", _locale);

    if (path.contains ("%1"))
    {
      for (char i ('2'); i <= '9'; ++i)
      {
        const QString replaced (path.arg (i));
        if (QFile::exists (replaced))
        {
          MPQArchive::loadMPQ (replaced.toStdString());
        }
      }
    }
    else
    {
      if (QFile::exists (path))
      {
        MPQArchive::loadMPQ (path.toStdString());
      }
    }
  }
}

void Noggit::create_world_view (World* world)
{
  MapView* map_view (new MapView (world, 0.0, 0.0, _dummy_gl_widget, NULL));

  const bool inFullscreen (_settings->value ("fullscreen").toBool());
  if (inFullscreen)
  {
    map_view->showFullScreen();
  }
  else
  {
    const int xResolution (_settings->value ("resolution/x").toInt());
    const int yResolution (_settings->value ("resolution/y").toInt());
    map_view->show();
    map_view->resize (xResolution, yResolution);
  }
}

void Noggit::add_font_from_mpq (const QString& filename) const
{
  const MPQFile file (filename);

  QFontDatabase::addApplicationFontFromData ( QByteArray::fromRawData ( file.getBuffer()
                                                                      , file.getSize()
                                                                      )
                                            );
}

int main( int argc, char *argv[] )
{
  Noggit application (argc, argv);

//! \todo remove vv

  const std::string arialFilename ("/Library/Fonts/arial.ttf");
  // Initializing Fonts
  skurri32.init( "fonts/skurri.ttf", 32, true );
  fritz16.init( "fonts/frizqt__.ttf", 16, true );
  morpheus40.init( "fonts/morpheus.ttf", 40, true );
  arialn13.init( "fonts/arialn.ttf", 13, true );

  arial12.init( arialFilename, 12, false );
  arial14.init( arialFilename, 14, false );
  arial16.init( arialFilename, 16, false );
  arial24.init( arialFilename, 24, false );
  arial32.init( arialFilename, 32, false );

  // init
  Environment::getInstance()->cursorColorR = 1.0f;
  Environment::getInstance()->cursorColorG = 1.0f;
  Environment::getInstance()->cursorColorB = 1.0f;
  Environment::getInstance()->cursorColorA = 1.0f;
  Environment::getInstance()->cursorType = 1;

  // load cursor settings
  if (QFile::exists ("NoggIt.conf"))
  {
    ConfigFile myConfigfile ( "NoggIt.conf" );
    if( myConfigfile.keyExists("RedColor") && myConfigfile.keyExists("GreenColor")  &&  myConfigfile.keyExists("BlueColor") &&  myConfigfile.keyExists("AlphaColor") )
    {
      Environment::getInstance()->cursorColorR = myConfigfile.read<float>( "RedColor" );
      Environment::getInstance()->cursorColorG = myConfigfile.read<float>( "GreenColor" );
      Environment::getInstance()->cursorColorB = myConfigfile.read<float>( "BlueColor" );
      Environment::getInstance()->cursorColorA = myConfigfile.read<float>( "AlphaColor" );
    }

  if( myConfigfile.keyExists("CursorType"))
    Environment::getInstance()->cursorType = myConfigfile.read<int>( "CursorType" );
  }

//! \todo remove ^^

  return application.exec();
}
