#ifndef __NOGGIT_MPQ_ARCHIVE_MANAGER_H
#define __NOGGIT_MPQ_ARCHIVE_MANAGER_H

#include <QString>
#include <QList>
#include <QPair>
#include <QMutex>
#include <QStringList>

class AsyncLoader;

namespace noggit
{
  namespace mpq
  {
    //! \note Instead of including StormLib.
    typedef void* HANDLE;

    class archive;

    class archive_manager
    {
    public:
      archive_manager (AsyncLoader*);
      ~archive_manager();

      void load_mpq (const QString& filename, bool process_list_file = false);
      void unload_all_mpqs();
      void unload_mpq (const QString& filename);

      bool file_exists_in_an_mpq (const QString& filename) const;

      void open_file_from_an_mpq ( const QString& filename
                                 , size_t* size
                                 , char** buffer
                                 );

      bool all_finished_loading() const;
      void all_finish_loading();

      const QStringList& listfile() const;
      void add_to_listfile (const QStringList& other);

    private:
      typedef QPair<QString, archive*> archive_entry_type;
      typedef QList<archive_entry_type> archives_type;
      archives_type _open_archives;

      AsyncLoader* _async_loader;

      QStringList _listfile;
      QMutex _listfile_mutex;
    };
  }
}

#endif

