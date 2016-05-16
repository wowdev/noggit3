// application.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Beket <snipbeket@mail.ru>
// Bernd L√∂rwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Mjolln√  <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

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
#include <QFileDialog>

#include <helper/repository.h>
#include <helper/qt/case_insensitive.h> //implizit cast

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapView.h>
#include <noggit/Menu.h>
#include <noggit/ModelManager.h> // ModelManager::report()
#include <noggit/mpq/archive.h>
#include <noggit/mpq/file.h>
#include <noggit/TextureManager.h> // TextureManager::report()
#include <noggit/ui/DBCEditor.h>
#include <noggit/ui/MainWindow.h>
#include <noggit/WMO.h> // WMOManager::report()

#include <csignal>
#include <string>
#ifdef Q_WS_WIN
#include <windows/StackWalker.h>
#else
#include <execinfo.h>
#endif

namespace noggit
{
  application::application (int& argc, char** argv)
  : QApplication (argc, argv)
  , _settings (nullptr)
  , _async_loader (1)
  , _archive_manager (_async_loader)
  {
    Log << "Noggit Studio - " << helper::repository::revision() << std::endl;

    setOrganizationDomain ("modcraft.tk");
    setOrganizationName ("Modcraft");
    setApplicationName ("Noggit");
    setGraphicsSystem ("opengl");

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

    mpq::file::disk_search_path (_project_path.absolutePath());
    open_mpqs();

    add_font_from_mpq ("fonts/skurri.ttf");
    add_font_from_mpq ("fonts/frizqt__.ttf");
    add_font_from_mpq ("fonts/morpheus.ttf");
    add_font_from_mpq ("fonts/arialn.ttf");

    OpenDBs();

    if (_settings->value ("use_styled_ui", false).toBool())
    {
      loadStyles();
    }

    initialize_shared_gl_widget();

    ui::MainWindow* mainwindow = new ui::MainWindow;
    if (_settings->value ("maximizedAppShow").toBool() == true)
      mainwindow->showMaximized();
    else
      mainwindow->show();
    mainwindow->map_selection_menu->raise();
    mainwindow->map_selection_menu->activateWindow();
  }

  mpq::archive_manager& application::archive_manager()
  {
    return _archive_manager;
  }

  async::loader& application::async_loader()
  {
    return _async_loader;
  }

  texture_manager& application::texture_manager()
  {
    return _texture_manager;
  }
  model_manager& application::model_manager()
  {
    return _model_manager;
  }
  wmo_manager& application::wmo_manager()
  {
    return _wmo_manager;
  }

  void application::initialize_shared_gl_widget()
  {
    if (!QGLFormat::hasOpenGL())
    {
      throw std::runtime_error ("Your system does not support OpenGL. Sorry, this application can't run without it.");
    }

    QGLFormat format;
    format.setStencilBufferSize (1);
    format.setDepthBufferSize (16);
    format.setAlphaBufferSize (8);

    if (setting("antialiasing").toBool())
    {
      format.setSampleBuffers (true);
      format.setSamples (4);
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
        GLenum const err (glewInit());
        if(GLEW_OK != err)
        {
          throw std::runtime_error
            ( "unable to initialize glew: "
            + std::string ((char const*)glewGetErrorString (err))
            );
        }

        //! \todo Fallback for old and bad platforms.
        if(!glGenBuffers)
        {
          glGenBuffers = glGenBuffersARB;
        }
        if(!glBindBuffer)
        {
          glBindBuffer = glBindBufferARB;
        }
        if(!glBufferData)
        {
          glBufferData = glBufferDataARB;
        }

        LogDebug << "GL: Version: " << glGetString (GL_VERSION) << std::endl;
        LogDebug << "GL: Vendor: " << glGetString (GL_VENDOR) << std::endl;
        LogDebug << "GL: Renderer: " << glGetString (GL_RENDERER) << std::endl;
      }
    };

    _shared_gl_widget = new dummy_gl_widget (format);
  }
  QGLWidget* application::shared_gl_widget() const
  {
    return _shared_gl_widget;
  }

  QVariant application::setting ( const QString& key
                                , const QVariant& value
                                ) const
  {
    return _settings->value (key, value);
  }

  void application::set_setting(const QString& key, const QVariant& value)
  {
    emit settingAboutToChange (key, setting (key));
    _settings->setValue (key, value);
    emit settingChanged (key, value);
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
    bool doMaximizedShow (_settings->value ("maximizedShow", true).toBool());
    bool doMaximizedAppShow (_settings->value ("maximizedAppShow", false).toBool());
    bool doProjectExplorerShow (_settings->value ("projectExplorerShow", false).toBool());
    qreal view_distance (_settings->value ("view_distance", 2048.0).toReal());

    foreach(const QString& argument, arguments())
    {
      if(argument == "-f" || argument == "--fullscreen")
        inFullscreen = true;
      else if(argument == "-noaa" || argument == "--noantialiasing")
        doAntiAliasing = false;
      else if(argument == "-nomaximizedshow" || argument == "--nomaximizedshow")
        doMaximizedShow = false;
      else if(argument == "-nomaximizedappshow" || argument == "--nomaximizedappshow")
        doMaximizedAppShow = false;
      else if(argument == "-projectexplorershow" || argument == "--projectexplorershow")
        doProjectExplorerShow = true;
      else
      {
        QRegExp resolution ("-(-resolution=)?(\\d+)(x(\\d+))*");

        if(!resolution.indexIn (argument))
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
    _settings->setValue ("maximizedShow", doMaximizedShow);
    _settings->setValue ("maximizedAppShow", doMaximizedAppShow);
    _settings->setValue ("projectExplorerShow", doProjectExplorerShow);
    _settings->setValue ("view_distance", view_distance);
    _settings->sync();
  }

  void application::auto_detect_game_path()
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

    if(_game_path.absolutePath() == "")
    {
      QSettings registry_win7 (win7_registry_path, QSettings::NativeFormat);
      _game_path = registry_win7.value ("InstallPath").toString();
    }

    if(_game_path.absolutePath() == "")
    {
      QSettings registry_win72 (win7_registry_path2, QSettings::NativeFormat);
      _game_path = registry_win72.value ("InstallPath").toString();
    }
#else
#ifdef Q_WS_MAC
    _game_path = "/Applications/World of Warcraft/";
#else
    _game_path = "";
#endif
#endif
  }

  bool application::is_valid_game_path (const QDir& path)
  {
    if (!path.exists())
    {
      LogError << "Path \"" << qPrintable (path.absolutePath())
               << "\" does not exist." << std::endl;
      return false;
    }

    QStringList locales;
    locales << "enGB" << "enUS" << "deDE" << "koKR" << "frFR"
            << "zhCN" << "zhTW" << "esES" << "esMX" << "ruRU";
    QString found_locale ("****");

    foreach(const QString& locale, locales)
    {
      if (path.exists (("Data/" + locale)))
      {
        found_locale = locale;
        break;
      }
    }

    if (found_locale == "****")
    {
      LogError << "Path \"" << qPrintable (path.absolutePath())
               << "\" does not contain a locale directory "
               << "(invalid installation or no installation at all)."
               << std::endl;
      return false;
    }

    if (app()._settings->value ("check_for_client_build", false).toBool())
    {
      //! \todo Do somehow else. This  also does not take patches into
      //! account and will always fail.
      mpq::archive archive ( path.absoluteFilePath ( "Data/"
                                                   + found_locale
                                                   + "/locale-"
                                                   + found_locale
                                                   + ".mpq"
                                                   )
                           , false
                           );

      char* buffer;
      size_t size;
      archive.open_file ( "component.wow-" + found_locale + ".txt"
                        , &size
                        , &buffer
                        );
      const QString component_file (buffer);

      const QRegExp version_regexp (".*version=\"(\\d+)\".*");
      version_regexp.exactMatch (component_file);

      const int client_build (version_regexp.cap (1).toInt());

      static const int build_3_3_5a (12340);

      if (client_build != build_3_3_5a)
      {
        LogError << "Path \"" << qPrintable (path.absolutePath())
                 << "\" does not include a client of version "
                 << build_3_3_5a << " but version "
                 << client_build << "." << std::endl;
        return false;
      }
    }

    return true;
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
      auto_detect_game_path();
    }

    while (!is_valid_game_path (_game_path))
    {
      _game_path = QFileDialog::getExistingDirectory
        (nullptr, tr("Open WoW Directory"), "/", QFileDialog::ShowDirsOnly);
      if (_game_path.absolutePath() == "")
      {
        LogError << "Could not auto-detect game path "
                 << "and user canceled the dialog." << std::endl;
        throw std::runtime_error ("no folder chosen");
      }
    }

    _project_path = _settings->value ("paths/project", _game_path.absolutePath()).toString();

    _locale = _settings->value ("locale", "****").toString();

    if(_locale == "****")
    {
      QStringList locales;
      locales << "enGB" << "enUS" << "deDE" << "koKR" << "frFR"
              << "zhCN" << "zhTW" << "esES" << "esMX" << "ruRU";

      foreach(const QString& locale, locales)
      {
        if (_game_path.exists (("Data/" + locale)))
        {
          _locale = locale;
          break;
        }
      }
    }

    _settings->setValue ("paths/game", _game_path.absolutePath());
    _settings->setValue ("paths/project", _project_path.absolutePath());
    _settings->setValue ("locale", _locale);
    _settings->sync();

    Log << "Game path: " << qPrintable (_game_path.absolutePath()) << std::endl;
    Log << "Project path: " << qPrintable (_project_path.absolutePath()) << std::endl;
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
      QString path (_game_path.absoluteFilePath ("Data/" + archive));
      path.replace ("{locale}", _locale);

      if(path.contains ("%1"))
      {
        for (char i ('2'); i <= '9'; ++i)
        {
          const QString replaced (path.arg (i));
          if (QFile::exists (replaced))
            _archive_manager.load_mpq (replaced, true);
        }
      }
      else
      {
        if(QFile::exists (path))
          _archive_manager.load_mpq (path, true);
      }
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

  void application::loadStyles()
  {
	  // load external style file
	  this->setStyle("plastique");

	  // set stylesheet file path
	  // check if file exists and if yes: Is it really a file and no directory?
	  QString stylePath = "style/noggit.style";
	  QFileInfo check_file(stylePath);
	  if (check_file.exists() && check_file.isFile())
	  {
		  QFile File(stylePath);
		  File.open(QFile::ReadOnly);
		  QString StyleSheet = QLatin1String(File.readAll());
		  this->setStyleSheet(StyleSheet);
	  }
  }

  application& app()
  {
    return *reinterpret_cast<application*> (qApp);
  }
}

int main (int argc, char* argv[]);

#ifdef Q_WS_WIN
int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
 return main (__argc, __argv);
}
#endif

namespace
{
  [[noreturn]] void print_error_and_exit (int sig)
  {
    // Reset to defaults.
    signal (SIGABRT, SIG_DFL);
    signal (SIGFPE, SIG_DFL);
    signal (SIGILL, SIG_DFL);
    signal (SIGSEGV, SIG_DFL);
    signal (SIGTERM, SIG_DFL);

    std::string description;
    std::string sign;

    switch (sig)
    {
      case SIGABRT:
        sign = "SIGABRT";
        description = "Abnormal termination, such as instigated by the abort function. (Abort.)";
        break;
      case SIGFPE:
        sign = "SIGFPE";
        description = "Erroneous arithmetic operation, such as divide by 0 or overflow. (Floating point exception.)";
        break;
      case SIGILL:
        sign = "SIGILL";
        description = "An 'invalid object program' has been detected. This usually means that there is an illegal instruction in the program. (Illegal instruction.)";
        break;
      case SIGINT:
        sign = "SIGINT";
        description = "Interactive attention signal; on interactive systems this is usually generated by typing some 'break-in' key at the terminal. (Interrupt.)";
        break;
      case SIGSEGV:
        sign = "SIGSEGV";
        description = "Invalid storage access; most frequently caused by attempting to store some value in an object pointed to by a bad pointer. (Segment violation.)";
        break;
      case SIGTERM:
        sign = "SIGTERM";
        description =  "Termination request made to the program. (Terminate.)";
        break;
      default:
        sign = "SIGUNK";
        description = "Unknown Exception!";
        break;
    }

    LogError << "There was an exception of type \""
             << sign
             << "\"\n\""
             << description
             << "\".\nPlease excuse the inconvenience. You may want to report this error including the log to the developers.\n"
             << std::endl;

#ifndef Q_WS_WIN
    static const int nframes (30);

    void* array[nframes];

    const size_t size (backtrace (array, nframes));
    char** strings (backtrace_symbols (array, size));

    LogError << "Obtained " << size << " stack frames." << std::endl;

    for (size_t i (0); i < size; ++i)
      LogError << "- " << strings[i] << std::endl;

    free (strings);
#else
    StackWalker sw;
    sw.ShowCallstack();
#endif

    exit (sig);
  }
}

int main (int argc, char* argv[])
{
  InitLogging();

#ifndef _DEBUG
  signal (SIGABRT, print_error_and_exit);
  signal (SIGFPE, print_error_and_exit);
  signal (SIGILL, print_error_and_exit);
  signal (SIGSEGV, print_error_and_exit);
  signal (SIGTERM, print_error_and_exit);
#endif

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
