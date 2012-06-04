// file.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Mjollnà <mjollna.wow@gmail.com>

#include <noggit/mpq/file.h>

#include <stdexcept>

#include <StormLib.h>

#include <noggit/Log.h>
#include <noggit/mpq/archive_manager.h>
#include <noggit/application.h>
#include <noggit/mpq/archive.h>

namespace noggit
{
  namespace mpq
  {
    QDir file::_disk_search_path;

    file::file (const QString& filename, const bool& maybe_create)
      : _is_at_end_of_file (true)
      , buffer (NULL)
      , pointer (0)
      , size (0)
      , _file_is_on_disk (false)
      , _filename (filename)
    {
      if (!exists (_filename))
      {
        if (!maybe_create)
        {
          LogError << "Requested file "
                   << qPrintable (_filename)
                   << " which does not exist."
                   << std::endl;

          throw std::runtime_error ("Requested file does not exist.");
        }
        else
        {
          _file_is_on_disk = true;
        }
      }
      else
      {
        if (_disk_search_path.exists (_filename))
        {
          QFile file (_disk_search_path.absoluteFilePath (_filename));
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
      }

      _is_at_end_of_file = size == 0;
    }

    void file::disk_search_path (const QDir& path)
    {
      _disk_search_path = path;
    }

    file::~file()
    {
      close();
    }

    bool file::exists ( const QString &filename)
    {
      return app().archive_manager().file_exists_in_an_mpq (filename)
          || _disk_search_path.exists (filename);
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
      delete buffer;
      buffer = NULL;

      buffer=Buf;
      size=Size;
    }

    char* file::getPointer() const
    {
      return buffer + pointer;
    }

    void file::save_to_disk()
    {
      const QString full_path
        (_disk_search_path.absoluteFilePath (_filename));

      const int pos (_filename.lastIndexOf ("/"));
      const QString filename (_filename.mid (pos));

      QDir dir (_disk_search_path);
      dir.cd (_filename.left (pos));

      if (!QDir().mkpath (dir.absolutePath()))
      {
        LogError << "Unable to create directory \""
                 << qPrintable (dir.absolutePath())
                 << "\". Saving might fail."
                 << std::endl;
      }

      QFile output_file (dir.absoluteFilePath (filename));

      if (output_file.open (QFile::WriteOnly))
      {
        Log << "Saving file \""
            << qPrintable (dir.absoluteFilePath (filename))
            << "\"."
            << std::endl;

        output_file.write (buffer, size);

        _file_is_on_disk = true;
      }
      else
      {
        LogError << "Unable to open \""
                 << qPrintable (dir.absoluteFilePath (filename))
                 << "\" for writing." << std::endl;
      }
    }

    void file::save_to_mpq (archive *arch, QString pathInMPQ)
    {

      if(!app().archive_manager().is_open(arch))
      {
          LogError << "Requested MPQ not open adding"<<std::endl;
      }


      if(pathInMPQ.isEmpty())
          pathInMPQ = "\\";

      pathInMPQ.replace("/", "\\");


      arch->add_file(this, pathInMPQ);

    }
  }
}

