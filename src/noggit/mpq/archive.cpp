#include <noggit/mpq/archive.h>

#include <StormLib.h>

#include <noggit/mpq/archive_manager.h>
#include <noggit/Log.h>
#include <noggit/application.h>

#include <QStringList>

#include <stdexcept>

namespace noggit
{
  namespace mpq
  {
    archive::archive (const QString& filename, bool process_list_file)
      : _archive_handle (NULL)
    {
      if ( !SFileOpenArchive ( filename.toLatin1().data()
                             , 0
                             , MPQ_OPEN_NO_LISTFILE
                             , &_archive_handle
                             )
         )
      {
        LogError << "Error opening archive: " << filename.toStdString() << "\n";
        throw std::runtime_error ("Opening archive failed.");
        return;
      }
      else
      {
        LogDebug << "Opened archive " << filename.toStdString() << "\n";
      }

      _finished = !process_list_file;
    }

    void archive::finish_loading()
    {
      if (_finished)
        return;

      //! \note We set this before doing something, as archive_manager relies on it being set for sorting after the last one.
      _finished = true;

      HANDLE file_handle;

      if (SFileOpenFileEx (_archive_handle, "(listfile)", 0, &file_handle))
      {
        const size_t filesize (SFileGetFileSize (file_handle));

        char* readbuffer (new char[filesize]);
        SFileReadFile (file_handle, readbuffer, filesize);
        SFileCloseFile (file_handle);

        app().archive_manager().add_to_listfile
          ( QString::fromAscii (readbuffer, filesize)
            .toLower()
            .split ( "\r\n"
                   , QString::SkipEmptyParts
                   )
          );

        delete[] readbuffer;
      }
    }

    archive::~archive()
    {
      if (_archive_handle)
      {
        SFileCloseArchive (_archive_handle);
      }
    }

    bool archive::has_file (QString filename) const
    {
      return SFileHasFile ( _archive_handle
                          , qPrintable (filename.replace ("/", "\\"))
                          );
    }

    bool archive::open_file ( const QString& filename
                            , size_t* size
                            , char** buffer
                            ) const
    {
      HANDLE file_handle;
      bool opened ( SFileOpenFileEx ( _archive_handle
                                    , qPrintable (filename)
                                    , 0
                                    , &file_handle
                                    )
                  );
      if (opened)
      {
        *size = SFileGetFileSize (file_handle);
        *buffer = new char[*size];

        SFileReadFile (file_handle, *buffer, *size);
        SFileCloseFile (file_handle);
      }

      return opened;
    }
  }
}
