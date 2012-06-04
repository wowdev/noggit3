// application.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
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

#include <noggit/DBC.h>
#include <noggit/errorHandling.h>
#include <noggit/Menu.h>
#include <noggit/MapView.h>
#include <noggit/TextureManager.h> // TextureManager::report()
#include <noggit/WMO.h> // WMOManager::report()
#include <noggit/ModelManager.h> // ModelManager::report()
#include <noggit/mpq/archive.h>
#include <noggit/mpq/file.h>
#include <noggit/ui/MainWindow.h>
#include <noggit/ui/DBCEditor.h>

namespace noggit
{
  application::application (int& argc, char** argv)
  : QApplication (argc, argv)
  , _settings (NULL)
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

    ui::MainWindow* mainwindow = new ui::MainWindow;
    if (_settings->value ("maximizedAppShow").toBool() == true)
      mainwindow->showMaximized();
    else
      mainwindow->show();
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

  QVariant application::setting(const QString& key) const
  {
    return _settings->value (key);
  }

  void application::setting(const QString& key, const QVariant& value)
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
    bool doMaximizedAppShow (_settings->value ("maximizedAppShow", true).toBool());
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
    _game_path.clear();
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

    if (_settings->value ("check_for_client_build", true).toBool())
    {
      //! \todo  Do somehow else  with not  loading and  unloading the
      //! MPQs multiple times. (Is that file in one specific one?)
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
        (NULL, tr("Open WoW Directory"), "/", QFileDialog::ShowDirsOnly);
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

int main (int argc, char* argv[])
{
#ifndef _DEBUG
  RegisterErrorHandlers();
#endif
  InitLogging();

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
