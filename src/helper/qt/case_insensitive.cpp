// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <helper/qt/case_insensitive.h>


namespace helper
{
  namespace qt
  {
    namespace case_insensitive
    {
      namespace
      {
        QString case_sensitive_equivalent (const QString& path)
		{
          const int pos (path.lastIndexOf (QRegExp("[\\\\/]")));
          const QString parent_path ( pos <= 0
                                    ? QDir::rootPath()
                                    : case_sensitive_equivalent
                                        (path.left (pos))
                                    );

          if (path.size() == 0)
          {
            return parent_path;
          }

          const QDir dir (parent_path);
          const QStringList files ( dir.entryList ( QDir::Dirs
                                                  | QDir::AllDirs
                                                  | QDir::Files
                                                  | QDir::Drives
                                                  | QDir::NoDotAndDotDot
                                                  | QDir::Hidden
                                                  | QDir::System
                                                  | QDir::AllEntries
                                                  )
                                  );


		  int const index(files.indexOf(QRegExp(path.mid(pos + 1)
			  , Qt::CaseInsensitive
			  )
			  ));

		  return dir.absoluteFilePath(index == -1 ? path.mid(pos + 1) : files.at(index));

        }
      }

      directory::directory (const QDir& dir)
        : QDir (case_sensitive_equivalent (dir.absolutePath()))
      { }

      directory::directory (const QString& path)
        : QDir (case_sensitive_equivalent (path))
      { }

      directory::directory ( const QString& path
                           , const QString& nameFilter
                           , QDir::SortFlags sort
                           , QDir::Filters filters
                           )
        : QDir ( case_sensitive_equivalent (path)
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
        : QFile (case_sensitive_equivalent (name))
      { }

      file::file (QObject* parent)
        : QFile (parent)
      { }

      file::file (const QString& name, QObject* parent)
        : QFile (case_sensitive_equivalent (name), parent)
      { }

    }
  }
}
