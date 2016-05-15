// file.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Mjollnà <mjollna.wow@gmail.com>

#ifndef __NOGGIT_MPQ_FILE_H
#define __NOGGIT_MPQ_FILE_H

#include <QString>

namespace helper
{
  namespace qt
  {
    namespace case_insensitive
    {
      class directory;
    }
  }
}

namespace noggit
{
  namespace mpq
  {
    //! \todo Use QDir for paths instead of a QString.
    //! \todo Put file on top of QFile?
    class archive;
    class file
    {
      Q_DISABLE_COPY (file);

    public:
      //! \note filenames are not case sensitive
      explicit file ( const QString& filename
                    , const bool& maybe_create = false
                    );
      ~file();

      size_t read (void* dest, size_t bytes);
      size_t getSize() const;
      size_t getPos() const;
      char* getBuffer() const;
      void setBuffer (char *Buf, size_t Size);
      char* getPointer() const;

      bool is_at_end_of_file() const;
      void seek (size_t offset);
      void seekRelative (size_t offset);
      void close();
      bool file_is_on_disk() const;


      template<typename T>
      const T* get (size_t offset) const
      {
        return reinterpret_cast<T*> (buffer + offset);
      }

      void save_to_disk();

      void save_to_mpq (archive *arch, QString pathInMPQ = nullptr);
      static bool exists (const QString& filename);
      static void disk_search_path
        (const helper::qt::case_insensitive::directory& path);

    private:
      bool _is_at_end_of_file;
      char* buffer;
      size_t pointer;
      size_t size;

      bool _file_is_on_disk;

      QString _filename;

      static helper::qt::case_insensitive::directory _disk_search_path;

      friend class archive;
    };

    std::string normalized_filename (std::string);
  }
}

#endif

