// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <vector>

namespace util
{
  //! \todo This name is horrible. The API is mediocre.
  class sExtendableArray
  {
    std::vector<char> data;

  public:
    //! Equivalent to `Allocate ($size + pAddition)`.
    void Extend (long pAddition);

    //! At \a pPosition, insert \a pAddition bytes that are all '\0',
    //! moving existing data further back.
    void Insert (unsigned long pPosition, unsigned long pAddition);

    //! At \a pPosition, insert \a pAddition bytes with the data at \a
    //! pAdditionalData, moving existing data further back.
    void Insert (unsigned long pPosition, unsigned long pAddition, const char * pAdditionalData);

    template<typename T>
      struct LazyPointer
    {
      LazyPointer (std::vector<char>& data, std::size_t position);
      T* get() const;
      T& operator*() const;
      T* operator->() const;
      T& operator[] (std::size_t) const;
      LazyPointer<T>& operator+= (std::size_t);

    private:
      std::vector<char>& _data;
      std::size_t _position;
    };

    template<typename To>
      LazyPointer<To> GetPointer(unsigned long pPosition = 0);

    std::vector<char> all_data() const;
    std::vector<char> data_up_to (std::size_t position) const;

    sExtendableArray() = default;
    sExtendableArray(unsigned long pSize, const char *pData);
  };
}

#include <util/sExtendableArray.ipp>
