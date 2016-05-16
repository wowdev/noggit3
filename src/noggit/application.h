// application.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd L√∂rwald <bloerwald+noggit@googlemail.com>
// Mjolln√  <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef __NOGGIT_APPLICATION_H
#define __NOGGIT_APPLICATION_H

#include <QApplication>
#include <QDir>
#include <QVariant>

#include <noggit/async/loader.h>
#include <noggit/ModelManager.h>
#include <noggit/mpq/archive_manager.h>
#include <noggit/TextureManager.h>
#include <noggit/WMO.h>

class QSettings;
class QGLWidget;

class World;

namespace noggit
{
  class application : public QApplication
  {
    Q_OBJECT

  public:
    application (int& argc, char** argv);

    QVariant setting ( const QString& key
                     , const QVariant& value = QVariant()
                     ) const;
    void set_setting (const QString& key, const QVariant& value);

    async::loader& async_loader();
    mpq::archive_manager& archive_manager();

    texture_manager& texture_manager();
    model_manager& model_manager();
    wmo_manager& wmo_manager();

    QGLWidget* shared_gl_widget() const;

  signals:
    void settingAboutToChange (const QString& key, const QVariant& value);
    void settingChanged (const QString& key, const QVariant& value);

  private:
    void set_working_directory_to_application_path();
    void parse_command_line_and_set_defaults();
    void get_game_path();
    void open_mpqs();
    void add_font_from_mpq (const QString& filename) const;
    void auto_detect_game_path();
    static bool is_valid_game_path (const QDir& path);
	void loadStyles();

    void initialize_shared_gl_widget();
    QGLWidget* _shared_gl_widget;

    QSettings* _settings;
    QDir _game_path;
    QDir _project_path;
    QString _locale;

    async::loader _async_loader;
    mpq::archive_manager _archive_manager;
    noggit::texture_manager _texture_manager;
    noggit::model_manager _model_manager;
    noggit::wmo_manager _wmo_manager;
  };

  application& app();
}

#endif
