// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QString>
#include <QList>
#include <QPair>
#include <QMutex>
#include <QStringList>

namespace noggit
{
  namespace async
  {
    class loader;
  }

  namespace mpq
  {
    //! \note Instead of including StormLib.
    typedef void* HANDLE;

    class archive;

    class archive_manager
    {
    public:
      archive_manager (async::loader&);
      ~archive_manager();

      archive *load_mpq (const QString& filename, bool process_list_file = false);
      archive *create_mpq (const QString& filename, bool process_list_file = false);
      void unload_all_mpqs();
      void unload_mpq (const QString& filename);

      bool file_exists_in_an_mpq (const QString& filename) const;

      void open_file_from_an_mpq ( const QString& filename
                                 , size_t* size
                                 , char** buffer
                                 );

      bool all_finished_loading() const;
      void all_finish_loading();
      bool is_open(archive* arch);

      const QStringList& listfile() const;
      void add_to_listfile (const QStringList& other);

    private:
      typedef QPair<QString, archive*> archive_entry_type;
      typedef QList<archive_entry_type> archives_type;
      archives_type _open_archives;

      async::loader& _async_loader;

      QStringList _listfile;
      QMutex _listfile_mutex;
    };
  }
}
