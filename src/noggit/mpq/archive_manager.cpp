// archive_manager.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Beket <snipbeket@mail.ru>
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/mpq/archive_manager.h>

#include <noggit/async/loader.h>
#include <noggit/Log.h>
#include <noggit/mpq/archive.h>

namespace noggit
{
  namespace mpq
  {
    archive_manager::archive_manager (async::loader& async_loader)
      : _open_archives()
      , _async_loader (async_loader)
      , _listfile()
    {
    }

    archive_manager::~archive_manager()
    {
      unload_all_mpqs();
    }

    const QStringList& archive_manager::listfile() const
    {
      return _listfile;
    }

    void archive_manager::add_to_listfile (const QStringList& other)
    {
      QMutexLocker locker (&_listfile_mutex);

      _listfile.append (other);

      if (all_finished_loading())
      {
        _listfile.sort();
        _listfile.removeDuplicates();

        LogDebug << "Completed listfile loading.\n";
      }
    }

    archive *archive_manager::load_mpq( const QString& filename
                                   , bool process_list_file
                                   )
    {
      archive* arch (new archive (filename, process_list_file));
      _open_archives.push_back (archive_entry_type (filename, arch));
      _async_loader.add_object (arch);
      return arch;
    }

    archive *archive_manager::create_mpq(const QString &filename
                                     , bool process_list_file
                                     )
    {
        archive* arch (new archive (filename, process_list_file, true));
        _open_archives.push_back (archive_entry_type (filename, arch));
        _async_loader.add_object (arch);
        return arch;
    }

    void archive_manager::unload_all_mpqs()
    {
      foreach (const archive_entry_type& entry, _open_archives)
      {
        delete entry.second;
      }
      _open_archives.clear();
    }

    void archive_manager::unload_mpq (const QString& filename)
    {
      for ( archives_type::iterator it (_open_archives.begin())
          , end (_open_archives.end())
          ; it != end
          ; ++it
          )
      {
        if (it->first == filename)
        {
          delete it->second;
          _open_archives.erase (it);
        }
      }
    }

    bool archive_manager::all_finished_loading() const
    {
      bool allFinished (true);
      foreach (const archive_entry_type& entry, _open_archives)
      {
        allFinished = allFinished && entry.second->finished_loading();
      }
      return allFinished;
    }

    void archive_manager::all_finish_loading()
    {
      foreach (const archive_entry_type& entry, _open_archives)
      {
        entry.second->finish_loading();
      }
    }

    bool archive_manager::file_exists_in_an_mpq (const QString& filename) const
    {
      foreach (const archive_entry_type& entry, _open_archives)
      {
        if (entry.second->has_file (filename))
        {
          return true;
        }
      }
      return false;
    }

    void archive_manager::open_file_from_an_mpq ( const QString& filename
                                                , size_t* size
                                                , char** buffer
                                                )
    {
      QListIterator<archive_entry_type> iterator (_open_archives);
      for (iterator.toBack(); iterator.hasPrevious(); iterator.previous())
      {
        if (iterator.peekPrevious().second->open_file (filename, size, buffer))
        {
          break;
        }
      }
    }

    bool archive_manager::is_open(archive* arch)
    {
        foreach (const archive_entry_type& entry, _open_archives)
        {
            if (entry.second == arch)
            {
                return true;
            }
        }
        return false;
    }

  }
}
