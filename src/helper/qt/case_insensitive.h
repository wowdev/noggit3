// non_recursive_filter_model.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#include <QString>
#include <QDir>
#include <QFile>

namespace helper
{
  namespace qt
  {
    namespace case_insensitive
    {
      namespace detail
      {
        QString case_sensitive_equivalent (const QString& path)
        {
          const int pos (path.lastIndexOf (QRegExp("[\\\\/]")));

          directory dir (ps < 0 ? QDir::rootPath() : path.left (pos));
          const QStringList files (dir.entryList());
          return dir.absoluteFilePath
            (files.at ( files.indexOf ( QRegExp ( path.mid (pos+1)
                                                , Qt::CaseInsensitive
                                                )
                                      )
                      )
            );
        }
      }

      class directory : public QDir
      {
      public:
        directory (const QDir & dir)
          : QDir (dir)
        { }
        directory (const QString& path = QString())
          : QDir (detail::case_sensitive_equivalent (path))
        { }
        directory ( const QString& path
                  , const QString& nameFilter
                  , QDir::SortFlags sort = QDir::SortFlags ( QDir::Name
                                                           | QDir::IgnoreCase
                                                           )
                  , QDir::Filters filters = QDir::AllEntries
                  )
          : QDir ( detail::case_sensitive_equivalent (path)
                 , nameFilter
                 , sort
                 , filters & ~QDir::CaseSensitive
                 )
        { }

        bool exists (const QString& name) const
        {
          if (QDir::exists (name))
          {
            return true;
          }

          QDir parent (*this);
          const QStringList dirNames (name.split (QRegExp("[\\\\/]")));

          foreach (QString dir, dirNames)
          {
              const QStringList entries (parent.entryList());
              if (!entries.contains (dir, Qt::CaseInsensitive))
              {
                return false;
              }

              parent.cd
                ( entrys.at ( entrys.indexOf ( QRegExp ( dir
                                                       , Qt::CaseInsensitive
                                                       )
                                             )
                            )
                );
          }

          return true;
        }
      };

      class file : public QFile
      {
      public:
        file (const QString& name)
          : QFile (detail::case_sensitive_equivalent (name))
        { }

        file (QObject* parent)
          : QFile (parent)
        { }

        file (const QString& name, QObject* parent)
          : QFile (detail::case_sensitive_equivalent (name), parent)
        { }
      };
    }
  }
}
