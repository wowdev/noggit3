// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <util/sExtendableArray.hpp>

namespace util
{
  void sExtendableArray::Extend (long pAddition)
  {
    data.resize (data.size() + pAddition);
  }

  void sExtendableArray::Insert (unsigned long pPosition, unsigned long pAddition)
  {
    std::vector<char> tmp (pAddition);
    data.insert (data.begin() + pPosition, tmp.begin(), tmp.end());
  }

  void sExtendableArray::Insert (unsigned long pPosition, unsigned long pAddition, const char * pAdditionalData)
  {
    data.insert (data.begin() + pPosition, pAdditionalData, pAdditionalData + pAddition);
  }

  std::vector<char> sExtendableArray::all_data() const
  {
    return data_up_to (data.size());
  }
  std::vector<char> sExtendableArray::data_up_to (std::size_t position) const
  {
    return std::vector<char> (data.begin(), data.begin() + position);
  }

  sExtendableArray::sExtendableArray(unsigned long pSize, const char *pData)
    : data (pData, pData + pSize)
  {}
}
