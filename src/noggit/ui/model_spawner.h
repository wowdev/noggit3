// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ModelView.h>
#include <QWidget>
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
      model_spawner (QWidget* parent = nullptr);

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
