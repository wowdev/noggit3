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
      : _archive_handle (NULL)
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
                LogError << "Error creating archive: " << filename.toStdString() << "\n";
                return;
            }
            else
            {
                LogDebug << "Created and Opened archive " << filename.toStdString() << "\n";
            }
        }else{
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
        _listfile = QString::fromAscii (readbuffer, filesize).toLower().split ("\r\n", QString::SkipEmptyParts);

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
        *size = SFileGetFileSize (file_handle);
        *buffer = new char[*size];

        SFileReadFile (file_handle, *buffer, *size);
        SFileCloseFile (file_handle);
      }

      return opened;
    }

    void archive::save_to_disk()
    {
        if(SFileCompactArchive(_archive_handle) && SFileCloseArchive(_archive_handle))
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
