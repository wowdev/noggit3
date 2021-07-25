// This file is part of Noggit3, licensed under GNU General Public License (version 3).

namespace util
{
  template<typename To>
    sExtendableArray::LazyPointer<To> sExtendableArray::GetPointer
      (unsigned long pPosition)
  {
    return {data, pPosition};
  }

  template<typename T>
      sExtendableArray::LazyPointer<T>::LazyPointer
        (std::vector<char>& data, std::size_t position)
    : _data (data)
    , _position (position)
  {}

  template<typename T>
    T* sExtendableArray::LazyPointer<T>::get() const
  {
    return reinterpret_cast<T*> (_data.data() + _position);
  }
  template<typename T>
    T& sExtendableArray::LazyPointer<T>::operator*() const
  {
    return *get();
  }
  template<typename T>
    T* sExtendableArray::LazyPointer<T>::operator->() const
  {
    return get();
  }
  template<typename T>
    T& sExtendableArray::LazyPointer<T>::operator[] (std::size_t i) const
  {
    return *(get() + i);
  }

  template<typename T>
    sExtendableArray::LazyPointer<T>&
      sExtendableArray::LazyPointer<T>::operator+= (std::size_t num)
  {
    _position += num * sizeof (T);
    return *this;
  }
}
