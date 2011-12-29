#include <noggit/mpq/file.h>

#include <QDir>

#include <stdexcept>

#include <StormLib.h>

#include <noggit/Log.h>
#include <noggit/mpq/archive_manager.h>
#include <noggit/application.h>

namespace noggit
{
  namespace mpq
  {
    QString file::_disk_search_path;

    file::file (const QString& filename)
      : _is_at_end_of_file (true)
      , buffer (NULL)
      , pointer (0)
      , size (0)
      , _file_is_on_disk (false)
    {
      if (!exists (filename))
      {
        LogError << "Requested file "
                 << qPrintable (filename)
                 << " which does not exist."
                 << std::endl;

        throw std::runtime_error ("Requested file does not exist.");
        return;
      }

      _path_on_disk = (_disk_search_path + filename).toLower();
      _path_on_disk.replace ("\\", "/");

      _file_is_on_disk = QFile::exists (_path_on_disk);

      if (_file_is_on_disk)
      {
        QFile file (_path_on_disk);
        file.open (QFile::ReadOnly);

        size = file.size();
        buffer = new char[size];

        memcpy (buffer, file.readAll().data(), size);
      }
      else
      {
        QString corrected_filename (filename);
        corrected_filename.replace ("/", "\\");

        app().archive_manager().open_file_from_an_mpq ( corrected_filename
                                                      , &size
                                                      , &buffer
                                                      );
      }

      _is_at_end_of_file = size == 0;
    }

    void file::disk_search_path (const QString& path)
    {
      _disk_search_path = path;
    }

    file::~file()
    {
      close();
    }

    bool file::exists (const QString& filename)
    {
      return app().archive_manager().file_exists_in_an_mpq (filename)
          || QFile::exists (_disk_search_path + filename);
    }

    size_t file::read (void* dest, size_t bytes)
    {
      if (_is_at_end_of_file)
        return 0;

      size_t rpos (pointer + bytes);
      if (rpos > size)
      {
        bytes = size - pointer;
        _is_at_end_of_file = true;
      }

      memcpy (dest, &(buffer[pointer]), bytes);

      pointer = rpos;

      return bytes;
    }

    bool file::is_at_end_of_file() const
    {
      return _is_at_end_of_file;
    }

    void file::seek(size_t offset)
    {
      pointer = offset;
      _is_at_end_of_file = (pointer >= size);
    }

    void file::seekRelative(size_t offset)
    {
      pointer += offset;
      _is_at_end_of_file = (pointer >= size);
    }

    void file::close()
    {
      delete[] buffer;
      buffer = NULL;

      _is_at_end_of_file = true;
    }

    size_t file::getSize() const
    {
      return size;
    }

    size_t file::getPos() const
    {
      return pointer;
    }

    bool file::file_is_on_disk() const
    {
      return _file_is_on_disk;
    }

    char* file::getBuffer() const
    {
      return buffer;
    }

    void file::setBuffer(char *Buf, size_t Size)
    {
      if(buffer)
      {
        delete buffer;
        buffer = NULL;
      }
      buffer=Buf;
      size=Size;
    }

    char* file::getPointer() const
    {
      return buffer + pointer;
    }

    void file::save_to_disk()
    {
      const QString file_name
        (_path_on_disk.mid (_path_on_disk.lastIndexOf ("/")));

      QDir dir (_path_on_disk.left (_path_on_disk.lastIndexOf ("/")));
      dir.makeAbsolute();
      const QString dir_name (dir.canonicalPath());

      if (!QDir().mkpath (dir_name))
      {

        LogError << "Is \""
                 << qPrintable (dir_name)
                 << "\" really a location I can write to?"
                 << std::endl;
      }

      QFile output_file (_path_on_disk);

      if (output_file.open (QFile::WriteOnly))
      {
        Log << "Saving file \""
            << qPrintable (_path_on_disk)
            << "\"."
            << std::endl;

        output_file.write (buffer, size);

        _file_is_on_disk = true;
      }
      else
      {
        LogError << "Unable to open that file for writing." << std::endl;
      }
    }

    void file::save_to_mpq (const QString& filename)
    {

      //! \todo Rewrite this.
      //! \todo Use const std::string& as parameter.
      //! \todo Get MPQ to save to via dialog or use development.MPQ.
      //! \todo Create MPQ nicer, if not existing.
      //! \todo Use a pointer to the archive instead of searching it by filename.
      //! \todo Format this code properly.
    /*

      static std::string modmpqpath="";//this will be the path to modders archive (with 'myworld' file inside)
      HANDLE mpq_a;
      if(modmpqpath=="")//create new user's mods MPQ
      {
        std::string newmodmpq (_disk_search_path);
        newmodmpq.append("Data\\patch-9.MPQ");
        SFileCreateArchive(newmodmpq.c_str(),MPQ_CREATE_ARCHIVE_V2 | MPQ_CREATE_ATTRIBUTES,0x40,&mpq_a);
        //! \note Is locale setting needed? LOCALE_NEUTRAL is windows only.
        SFileSetFileLocale(mpq_a,0); // 0 = LOCALE_NEUTRAL.
        SFileAddFileEx(mpq_a,"shaders\\terrain1.fs","myworld",MPQ_FILE_COMPRESS,MPQ_COMPRESSION_ZLIB);//I must to add any file with name "myworld" so I decided to add terrain shader as "myworld"
        SFileCompactArchive(mpq_a);
        SFileCloseArchive(mpq_a);
        modmpqpath=newmodmpq;
      }
      else
        archive::unloadMPQ (QString::fromStdString (modmpqpath));

      SFileOpenArchive(modmpqpath.c_str(), 0, 0, &mpq_a );
      SFileSetLocale(0);
      std::string nameInMPQ = filename;
      nameInMPQ.erase(0,_disk_search_path.length());
      size_t found = nameInMPQ.find( "/" );
      while( found != std::string::npos )//fixing path to file
      {
        nameInMPQ.replace( found, 1, "\\" );
        found = nameInMPQ.find( "/" );
      }
      if(SFileAddFileEx(mpq_a,filename,nameInMPQ.c_str(),MPQ_FILE_COMPRESS|MPQ_FILE_ENCRYPTED|MPQ_FILE_REPLACEEXISTING,MPQ_COMPRESSION_ZLIB))
      {LogDebug << "Added file "<<_path_on_disk.c_str()<<" to archive \n";} else LogDebug << "Error "<<GetLastError()<< " on adding file to archive! Report this message \n";
      SFileCompactArchive(mpq_a);//recompact our archive to avoid fragmentation
      SFileCloseArchive(mpq_a);
      new MPQArchive(QString::fromStdString (modmpqpath), true);//now load edited archive to memory again
      */
    }
  }
}

