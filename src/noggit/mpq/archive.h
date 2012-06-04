// archive.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#ifndef __NOGGIT_MPQ_ARCHIVE_H
#define __NOGGIT_MPQ_ARCHIVE_H

#include <QString>
#include <QStringList>

#include <noggit/async/object.h>

namespace noggit
{
  class application;
  namespace mpq
  {
    //! \note Instead of including StormLib.
    typedef void* HANDLE;

    class archive_manager;
    class file;

    class archive : public async::object
    {
    public:
      virtual ~archive();

      bool has_file (QString filename) const;
      bool open_file ( const QString& filename
                     , size_t* size
                     , char** buffer
                     ) const;

      void finish_loading();
      void save_to_disk();
      void add_file(file* file, QString pathInMPQ);
      const QStringList& listfile() const;

    private:
      archive (const QString& filename, bool process_list_file, bool create_mpq = false);

      HANDLE _archive_handle;
      QStringList _listfile;

      friend class archive_manager;
      friend class noggit::application;
    };
  }
}

#endif

