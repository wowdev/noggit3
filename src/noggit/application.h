// application.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef __NOGGIT_APPLICATION_H
#define __NOGGIT_APPLICATION_H

#include <QApplication>

#include <noggit/async/loader.h>
#include <noggit/mpq/archive_manager.h>

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
    ~application();

    async::loader& async_loader();
    mpq::archive_manager& archive_manager();

  public slots:
    void create_world_view (World*);

  private:
    void set_working_directory_to_application_path();
    void parse_command_line_and_set_defaults();
    void initialize_video();
    void get_game_path();
    void open_mpqs();
    void add_font_from_mpq (const QString& filename) const;

    QSettings* _settings;
    QString _game_path;
    QString _project_path;
    QString _locale;

    QGLWidget* _dummy_gl_widget;

    async::loader _async_loader;
    mpq::archive_manager _archive_manager;
  };

  application& app();
}

#endif
