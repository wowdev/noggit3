// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Log.h>
#include <noggit/mpq/file.h>

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
      std::string const normalized (_normalize (filename));
      if (--_counts.at (normalized) == 0)
      {
        _elements.erase (normalized);
        _counts.erase (normalized);
      }
    }

    void apply (std::function<void (std::string const&, T&)> fun)
    {
      for (auto& element : _elements)
      {
        fun (element.first, element.second);
      }
    }
    void apply (std::function<void (std::string const&, T const&)> fun) const
    {
      for (auto const& element : _elements)
      {
        fun (element.first, element.second);
      }
    }

  private:
    std::map<std::string, T> _elements;
    std::unordered_map<std::string, std::size_t> _counts;
    std::function<std::string (std::string)> _normalize;
  };
}
