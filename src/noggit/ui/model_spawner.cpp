// model_spawner.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>

#include <noggit/ui/model_spawner.h>

#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRegExp>
#include <QStandardItemModel>
#include <QMimeData>

#include <helper/qt/non_recursive_filter_model.h>

#include <noggit/application.h>
#include <noggit/Log.h>

namespace noggit
{
  namespace ui
  {
    namespace detail
    {
      class model_tree_model : public QStandardItemModel
      {
      public:
        model_tree_model (QObject* parent = NULL)
          : QStandardItemModel (parent)
        { }

        virtual QStringList mimeTypes() const
        {
          return QStringList() << model_spawner::mime_type();
        }

        virtual QMimeData* mimeData (const QModelIndexList& indexes) const
        {
          QString path ("");
          QModelIndex index (indexes.first());
          while (index != QModelIndex())
          {
            path.prepend (index.data().toString()).prepend ("/");
            index = index.parent();
          }
          path = path.mid (1);

          //! \todo Just look that in up in the first index..
          if (path.endsWith (".m2") || path.endsWith (".wmo"))
          {
            QMimeData* mime (new QMimeData);
            mime->setData (model_spawner::mime_type(), path.toUtf8());
            return mime;
          }
          else
          {
            return NULL;
          }
        }
      };
    }

    model_spawner::model_spawner (QWidget* parent)
      : QWidget (parent)
      , _tree_model (new helper::qt::non_recursive_filter_model (NULL))
      , _file_tree (new QTreeView (NULL))
    {
      QVBoxLayout* layout (new QVBoxLayout (this));

      QLineEdit* filter_line (new QLineEdit (this));

      //! \todo Use lookahead assertion?
      QStringList listfile ( app()
                           .archive_manager()
                           .listfile()
                           .filter (QRegExp ("\\.(m2|wmo)$", Qt::CaseInsensitive))
                           );
      const QStringList filtered_file ( listfile.toSet()
                                      .subtract ( listfile
                                                .filter (QRegExp ("_[0-9][0-9][0-9]\\.wmo$"))
                                                .toSet()
                                                )
                                      .toList()
                                      );

      detail::model_tree_model* model (new detail::model_tree_model (this));

      QMap<QString, QStandardItem*> items;
      items[""] = model->invisibleRootItem();
      foreach (const QString& file, filtered_file)
      {
        const QStringList directories (file.left (qMax (file.lastIndexOf ("\\"), 0)).split ("\\"));
        QString path ("");

        foreach (const QString& dir, directories)
        {
          const QString new_path (path + dir + "\\");
          QStandardItem* item (items[new_path]);
          if (!item)
          {
            item = new QStandardItem (dir);
            items[path]->appendRow (item);
            items[new_path] = item;
          }
          path = new_path;
        }

        QStandardItem* child (new QStandardItem (file.mid (qMax (file.lastIndexOf ("\\") + 1, 0))));
        child->setEditable (false);

        items[path]->appendRow (child);
      }

      _tree_model->setSourceModel (model);
      _file_tree->setModel (_tree_model);
      _file_tree->setSortingEnabled (true);
      _file_tree->sortByColumn (0, Qt::AscendingOrder);
      _file_tree->setDragEnabled (true);
      _file_tree->setDragDropMode (QAbstractItemView::DragOnly);

      connect (filter_line, SIGNAL (textChanged (const QString&)), SLOT (update_filter (const QString&)));

      layout->addWidget (filter_line);
      layout->addWidget (_file_tree);

      //! \todo Add a gl widget with a preview of the selected model?
    }

    void model_spawner::update_filter (const QString& filter)
    {
      _tree_model->setFilterRegExp (QRegExp (filter, Qt::CaseInsensitive));
    }

    const QLatin1String& model_spawner::mime_type()
    {
      static const QLatin1String type ("noggit/model_path");
      return type;
    }
  }
}
