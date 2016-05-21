// archive.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <noggit/mpq/archive.h>

#include <StormLib.h>

#include <noggit/mpq/archive_manager.h>
#include <noggit/mpq/file.h>
#include <noggit/Log.h>
#include <noggit/application.h>

#include <QStringList>

#include <stdexcept>

namespace noggit
{
  namespace mpq
  {
    archive::archive (const QString& filename, bool process_list_file, bool create_mpq)
      : _archive_handle (nullptr)
    {
        if (create_mpq)
        {
            if ( !SFileCreateArchive ( (const char *)filename.toLatin1().data()
                                        , MPQ_CREATE_ARCHIVE_V2 | MPQ_CREATE_ATTRIBUTES
                                        , 0x40
                                        , &_archive_handle
                                        )
                 )
            {
              throw std::runtime_error ("Error creating archive: " + filename.toStdString());
            }
            else
            {
                LogDebug << "Created and Opened archive " << filename.toStdString() << "\n";
            }
        }else{
            if ( !SFileOpenArchive ( filename.toLatin1().data()
                                     , 0
                                     , MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY
                                     , &_archive_handle
                                     )
                 )
            {
              throw std::runtime_error ("Error opening archive: " + filename.toStdString());
            }
            else
            {
                LogDebug << "Opened archive " << filename.toStdString() << "\n";
            }
        }


      _finished = !process_list_file;
    }

    namespace
    {
      //! \note This all does not work with files > 2^32 at all.
      std::size_t get_file_size (HANDLE file_handle)
      {
		DWORD filesize_high (0);
        const unsigned int filesize_low (SFileGetFileSize (file_handle, &filesize_high));
        return filesize_low | size_t (filesize_high) << 32;
      }

      void read_file (HANDLE file_handle, void* buffer, std::size_t size)
      {
		DWORD read (0);
        SFileReadFile (file_handle, buffer, size, &read, nullptr);
        if (read != size)
        {
          throw std::logic_error ("read less than filesize");
        }
      }
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
        const size_t filesize (get_file_size (file_handle));

        char* readbuffer (new char[filesize]);
        read_file (file_handle, readbuffer, filesize);
        SFileCloseFile (file_handle);
        _listfile = QString::fromLatin1(readbuffer, filesize).toLower().split ("\r\n", QString::SkipEmptyParts);

        app().archive_manager().add_to_listfile(_listfile);

        delete[] readbuffer;
      }
    }

    const QStringList& archive::listfile() const
    {
      return _listfile;
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
        *size = get_file_size (file_handle);
        *buffer = new char[*size];

        read_file (file_handle, *buffer, *size);
        SFileCloseFile (file_handle);
      }

      return opened;
    }

    void archive::save_to_disk()
    {
        if(SFileCompactArchive(_archive_handle, nullptr, false) && SFileCloseArchive(_archive_handle))
            LogDebug << "Saved MPQ to disk" << std::endl;
        else
            LogError << "Error: " << GetLastError()<< "while saving MPQ to disk" << std::endl;
    }

    void archive::add_file(file* file, QString pathInMPQ)
    {
        HANDLE file_handle;

        if(SFileCreateFile(this->_archive_handle
                           ,pathInMPQ.toStdString().c_str()
                           ,0
                           ,file->getSize()
                           ,0
                           ,MPQ_FILE_COMPRESS|MPQ_FILE_ENCRYPTED|MPQ_FILE_REPLACEEXISTING
                           ,&file_handle))
            LogDebug << "Created file" << pathInMPQ.toStdString() << "wthin MPQ"<<std::endl;
        else{
            LogError << "Error: " << GetLastError()<< "while creatin file '"<<pathInMPQ.toStdString()<<"' within MPQ" << std::endl;
            return;
        }

        if(SFileWriteFile(file_handle
                          ,file->getBuffer()
                          ,sizeof(file->getBuffer())
                          ,MPQ_COMPRESSION_ZLIB))
            LogDebug << "Wrote data to file" << pathInMPQ.toStdString() << "wthin MPQ"<<std::endl;
        else{
            LogError << "Error: " << GetLastError()<< "while writing data to file '"<<pathInMPQ.toStdString()<<"' within MPQ" << std::endl;
            return;
        }

        if(SFileFinishFile(file_handle))
            LogDebug << "Finished file" << pathInMPQ.toStdString() << "wthin MPQ"<<std::endl;
        else{
            LogError << "Error: " << GetLastError()<< "while finishing file '"<<pathInMPQ.toStdString()<<"' within MPQ" << std::endl;
            return;
        }
    }

  }
}
