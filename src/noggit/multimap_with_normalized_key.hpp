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
    struct multimap_with_normalized_key
  {
    multimap_with_normalized_key (std::function<std::string (std::string)> normalize = &mpq::normalized_filename)
      : _normalize (std::move (normalize))
    {}

    ~multimap_with_normalized_key()
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
      boost::mutex::scoped_lock lock(_mutex);
      std::string const normalized (_normalize (filename));
      if (_counts[normalized]++ == 0)
      {
        return &_elements.emplace ( std::piecewise_construct
                                  , std::forward_as_tuple (normalized)
                                  , std::forward_as_tuple (normalized, args...)
                                  ).first->second;
      }
      return &_elements.at (normalized);
    }
    void erase (std::string const& filename)
    {
      boost::mutex::scoped_lock lock(_mutex);
      std::string const normalized (_normalize (filename));
      if (--_counts.at (normalized) == 0)
      {
        _elements.erase (normalized);
        _counts.erase (normalized);
      }
    }

    std::size_t count(std::string const& filename)
    {
      boost::mutex::scoped_lock lock(_mutex);
      std::string const normalized (_normalize (filename));

      auto const& it = _counts.find(normalized);
      return it == _counts.end() ? 0 : it->second;
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

  template<typename T>
    struct async_object_multimap_with_normalized_key
  {
      async_object_multimap_with_normalized_key (std::function<std::string (std::string)> normalize = &mpq::normalized_filename)
        : _map(normalize)
      {}

      template<typename... Args>
      T* emplace (std::string const& filename, Args&&... args)
      {
        T* obj = _map.emplace(filename, args...);

        // new object added
        if (_map.count(filename) == 1)
        {
          AsyncObject* async_obj = static_cast<AsyncObject*>(obj);
          AsyncLoader::instance().queue_for_load(async_obj);
        }        

        return obj;
      }

      void erase (std::string const& filename)
      {
        _map.erase(filename);
      }

      void apply (std::function<void (std::string const&, T&)> fun)
      {
        _map.apply(fun);
      }
      void apply (std::function<void (std::string const&, T const&)> fun) const
      {
        _map.apply(fun);
      }

  private:
      multimap_with_normalized_key<T> _map;
  };
}
