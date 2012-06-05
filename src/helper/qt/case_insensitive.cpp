// case_insensitive.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#include <helper/qt/case_insensitive.h>

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

          //loops till its death without
          if(pos < 0) return QDir::rootPath();

          directory dir (path.left (pos));
          const QStringList files (dir.entryList());
          return dir.absoluteFilePath
            (files.at ( files.indexOf ( QRegExp ( path.mid (pos + 1)
                                                , Qt::CaseInsensitive
                                                )
                                      )
                      )
            );
        }
      }

      directory::directory (const QDir & dir)
        : QDir (dir)
      { }

      directory::directory (const QString& path)
        : QDir (detail::case_sensitive_equivalent (path))
      { }

      directory::directory ( const QString& path
                , const QString& nameFilter
                , QDir::SortFlags sort
                , QDir::Filters filters
                  )
        : QDir ( detail::case_sensitive_equivalent (path)
               , nameFilter
               , sort
               , filters & ~QDir::CaseSensitive
               )
      { }

      bool directory::exists (const QString& name) const
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
            ( entries.at ( entries.indexOf ( QRegExp ( dir
                                                     , Qt::CaseInsensitive
                                                     )
                                           )
                         )
            );
        }

        return true;
      }


      file::file (const QString& name)
        : QFile (detail::case_sensitive_equivalent (name))
      { }

      file::file (QObject* parent)
        : QFile (parent)
      { }

      file::file (const QString& name, QObject* parent)
        : QFile (detail::case_sensitive_equivalent (name), parent)
      { }

    }
  }
}
