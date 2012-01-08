#include <noggit/application.h>

#include <stdexcept>

#include <opengl/types.h>

#include <QDir>
#include <QSettings>
#include <QTime>
#include <QGLWidget>
#include <QGLFormat>
#include <QFontDatabase>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include <helper/repository.h>

#include <noggit/DBC.h>
#include <noggit/errorHandling.h>
#include <noggit/Menu.h>
#include <noggit/MapView.h>
#include <noggit/TextureManager.h> // TextureManager::report()
#include <noggit/WMO.h> // WMOManager::report()
#include <noggit/ModelManager.h> // ModelManager::report()
#include <noggit/mpq/file.h>

namespace noggit
{
  application::application (int& argc, char** argv)
    : QApplication (argc, argv)
    , _settings (NULL)
    , _async_loader (1)
    , _archive_manager (_async_loader)
  {
#ifndef _DEBUG
    RegisterErrorHandlers();
#endif
    InitLogging();
    Log << "Noggit Studio - " << helper::repository::revision() << std::endl;

    setOrganizationDomain ("modcraft.tk");
    setOrganizationName ("Modcraft");
    setApplicationName ("Noggit");
    setApplicationVersion (helper::repository::revision_string());

    _settings = new QSettings (this);

    QTranslator* qtTranslator (new QTranslator (this));
    qtTranslator->load ("qt_" + QLocale::system().name(), QLibraryInfo::location (QLibraryInfo::TranslationsPath));
    installTranslator (qtTranslator);

    QTranslator* appTranslator (new QTranslator (this));
    appTranslator->load ("noggit_" + QLocale::system().name());
    installTranslator (appTranslator);

    qsrand (QTime::currentTime().msec());

    set_working_directory_to_application_path();
    parse_command_line_and_set_defaults();
    get_game_path();

    mpq::file::disk_search_path (_project_path);
    open_mpqs();

    add_font_from_mpq ("fonts/skurri.ttf");
    add_font_from_mpq ("fonts/frizqt__.ttf");
    add_font_from_mpq ("fonts/morpheus.ttf");
    add_font_from_mpq ("fonts/arialn.ttf");

    OpenDBs();

    initialize_video();

    Menu* map_selection_menu (new Menu (NULL));
    connect (map_selection_menu, SIGNAL (create_world_view_request (World*)), SLOT (create_world_view (World*)));
    map_selection_menu->show();
  }

  application::~application()
  {
    TextureManager::report();
    ModelManager::report();
    WMOManager::report();
  }

  mpq::archive_manager& application::archive_manager()
  {
    return _archive_manager;
  }

  async::loader& application::async_loader()
  {
    return _async_loader;
  }

  void application::set_working_directory_to_application_path()
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

  void application::parse_command_line_and_set_defaults()
  {
    int xResolution (_settings->value ("resolution/x", 1024).toInt());
    int yResolution (_settings->value ("resolution/y", 768).toInt());
    bool inFullscreen (_settings->value ("fullscreen", false).toBool());
    bool doAntiAliasing (_settings->value ("antialiasing", true).toBool());
    qreal view_distance (_settings->value ("view_distance", 2048.0).toReal());

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

      //! \todo Fallback for old and bad platforms.
      if (!glGenBuffers) glGenBuffers = glGenBuffersARB;
      if (!glBindBuffer) glBindBuffer = glBindBufferARB;
      if (!glBufferData) glBufferData = glBufferDataARB;

      LogDebug << "GL: Version: " << glGetString (GL_VERSION) << std::endl;
      LogDebug << "GL: Vendor: " << glGetString (GL_VENDOR) << std::endl;
      LogDebug << "GL: Renderer: " << glGetString (GL_RENDERER) << std::endl;
    }
  };

  void application::initialize_video()
  {
    if (!QGLFormat::hasOpenGL())
    {
      LogError << "Your system does not support OpenGL. Sorry, this application can't run without it." << std::endl;
    }

    QGLFormat format;
    format.setStencilBufferSize (1);
    format.setDepthBufferSize (16);
    format.setAlphaBufferSize (8);

    if (_settings->value ("antialiasing").toBool())
    {
      format.setSampleBuffers (true);
      format.setSamples (4);
    }

    _dummy_gl_widget = new dummy_gl_widget (format);
  }

  void application::get_game_path()
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
      static const QString win7_registry_path
        ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Blizzard Entertainment\\World of Warcraft");
      static const QString win7_registry_path2
        ("HKEY_CURRENT_USER\\Software\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\Wow6432Node\\Blizzard Entertainment\\World of Warcraft"); // path if you never installed wow under win7

      QSettings registry (default_registry_path, QSettings::NativeFormat);
      _game_path = registry.value ("InstallPath").toString();

      if(_game_path=="")
      {
        QSettings registry_win7 (win7_registry_path, QSettings::NativeFormat);
        _game_path = registry_win7.value ("InstallPath").toString();
      }

      if(_game_path=="")
      {
        QSettings registry_win72 (win7_registry_path2, QSettings::NativeFormat);
        _game_path = registry_win72.value ("InstallPath").toString();
      }
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

  void application::open_mpqs()
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
            _archive_manager.load_mpq (replaced, true);
          }
        }
      }
      else
      {
        if (QFile::exists (path))
        {
          _archive_manager.load_mpq (path, true);
        }
      }
    }
  }

  void application::create_world_view (World* world)
  {
    MapView* map_view ( new MapView ( world
                                    , _settings->value ("view_distance").toReal()
                                    , 0.0
                                    , 0.0
                                    , _dummy_gl_widget
                                    , NULL
                                    )
                      );

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

  void application::add_font_from_mpq (const QString& filename) const
  {
    const mpq::file file (filename);

    QFontDatabase::addApplicationFontFromData
      ( QByteArray::fromRawData ( file.getBuffer()
                                , file.getSize()
                                )
      );
  }

  application& app()
  {
    return *reinterpret_cast<application*> (qApp);
  }
}

int main (int argc, char *argv[]);

#ifdef Q_WS_WIN
int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
 return main (__argc, __argv);
}
#endif

int main (int argc, char *argv[])
{
  try
  {
    noggit::application application (argc, argv);

    return application.exec();
  }
  catch (const std::exception& e)
  {
    LogError << "Unrecoverable error: " << e.what() << std::endl;
  }
}
