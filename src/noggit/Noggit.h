#ifndef NOGGIT_H
#define NOGGIT_H

#include <QApplication>

class QSettings;
class QGLWidget;

class AsyncLoader;
class World;

class Noggit : public QApplication
{
  Q_OBJECT

public:
  Noggit (int& argc, char** argv);
  ~Noggit();

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
  AsyncLoader* _async_loader;
  QString _game_path;
  QString _project_path;
  QString _locale;

  QGLWidget* _dummy_gl_widget;
};

//! \todo remove below.
namespace freetype { class font_data; }
extern freetype::font_data arialn13, arial12, arial14, arial16, arial24, arial32, morpheus40, skurri32, fritz16;
extern AsyncLoader* gAsyncLoader;

#endif
