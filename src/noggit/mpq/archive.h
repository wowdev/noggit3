#ifndef __NOGGIT_MPQ_ARCHIVE_H
#define __NOGGIT_MPQ_ARCHIVE_H

#include <QString>

#include <noggit/async/object.h>

namespace noggit
{
  namespace mpq
  {
    //! \note Instead of including StormLib.
    typedef void* HANDLE;

    class archive_manager;

    class archive : public async::object
    {
    public:
      ~archive();

      bool has_file (QString filename) const;
      bool open_file ( const QString& filename
                     , size_t* size
                     , char** buffer
                     ) const;

      void finish_loading();

    private:
      archive (const QString& filename, bool process_list_file);

      HANDLE _archive_handle;

      friend class archive_manager;
    };
  }
}

#endif

