// case_insensitive.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#pragma once

#include <QString>
#include <QDir>
#include <QFile>

namespace helper
{
  namespace qt
  {
    namespace case_insensitive
    {

      class directory : public QDir
      {
      public:
        directory (const QDir & dir);
        directory (const QString& path = QString());
        directory ( const QString& path
                  , const QString& nameFilter
                  , QDir::SortFlags sort = QDir::SortFlags ( QDir::Name
                                                           | QDir::IgnoreCase
                                                           )
                  , QDir::Filters filters = QDir::AllEntries
                  );

        bool exists (const QString& name) const;
      };

      class file : public QFile
      {
      public:
        file (const QString& name);
        file (QObject* parent);
        file (const QString& name, QObject* parent);
      };
    }
  }
}
