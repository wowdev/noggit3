// model_spawner.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>

#ifndef __NOGGIT_UI_MODEL_SPAWNER_H
#define __NOGGIT_UI_MODEL_SPAWNER_H
#include <noggit/ModelView.h>
#include <QWidget>
#include <QGLWidget>
#include <QModelIndex>


class QTreeView;

namespace helper
{
  namespace qt
  {
    class non_recursive_filter_model;
  }
}

namespace noggit
{
  namespace ui
  {
    class model_spawner : public QWidget
    {
    Q_OBJECT

    public:
      model_spawner (QWidget* parent = NULL, QGLWidget *shared = NULL);

      static const QLatin1String& mime_type();

    private slots:
      void update_filter (const QString& filter);
      void changeModel(QModelIndex index);

    private:
      helper::qt::non_recursive_filter_model* _tree_model;
      QTreeView* _file_tree;
      ModelView* modelview;
    };
  }
}

#endif
