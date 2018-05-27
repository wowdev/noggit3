// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncLoader.h>
#include <noggit/AsyncObject.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>

#include <boost/thread.hpp>

#include <functional>
#include <map>
#include <string>
#include <unordered_map>

namespace noggit
{
  template<typename T>
    struct async_object_multimap_with_normalized_key
  {
    async_object_multimap_with_normalized_key (std::function<std::string (std::string)> normalize = &mpq::normalized_filename)
      : _normalize (std::move (normalize))
    {}

    ~async_object_multimap_with_normalized_key()
    {
      apply ( [&] (std::string const& key, T const&)
              {
                LogDebug << key << ": " << _counts.at (key) << "\n";
              }
            );
    }

    template<typename... Args>
      T* emplace (std::string const& filename, Args&&... args)
    {
      std::string const normalized (_normalize (filename));
      std::size_t count;

      {
        boost::mutex::scoped_lock lock(_mutex);
        count = _counts[normalized]++;
      }

      if (count == 0)
      {
        T* obj;

        {
          boost::mutex::scoped_lock lock(_mutex);
          obj = &_elements.emplace ( std::piecewise_construct
                                   , std::forward_as_tuple (normalized)
                                   , std::forward_as_tuple (normalized, args...)
                                   ).first->second;
        }        

        AsyncLoader::instance().queue_for_load(static_cast<AsyncObject*>(obj));

        return obj;
      }

      return &_elements.at (normalized);
    }
    void erase (std::string const& filename)
    {
      std::size_t count;      
      std::string const normalized (_normalize (filename));

      {
        boost::mutex::scoped_lock lock(_mutex);
        count = --_counts.at (normalized);
      }

      if (count == 0)
      {
        // always make sure an async object can be deleted before deleting it
        AsyncLoader::instance().ensure_deletable(static_cast<AsyncObject*>(&_elements.at(normalized)));

        {
          boost::mutex::scoped_lock lock(_mutex);
          _elements.erase (normalized);
          _counts.erase (normalized);
        }
      }
    }
    void apply (std::function<void (std::string const&, T&)> fun)
    {
      boost::mutex::scoped_lock lock(_mutex);
      for (auto& element : _elements)
      {
        fun (element.first, element.second);
      }
    }
    void apply (std::function<void (std::string const&, T const&)> fun) const
    {
      boost::mutex::scoped_lock lock(_mutex);
      for (auto const& element : _elements)
      {
        fun (element.first, element.second);
      }
    }

  private:
    std::map<std::string, T> _elements;
    std::unordered_map<std::string, std::size_t> _counts;
    std::function<std::string (std::string)> _normalize;
    boost::mutex _mutex;
  };
}
