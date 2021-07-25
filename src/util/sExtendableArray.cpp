// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <util/sExtendableArray.hpp>

namespace util
{
  void sExtendableArray::Allocate (unsigned long pSize)
  {
    data.resize (pSize);
  }

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
    return data;
  }

  sExtendableArray::sExtendableArray(unsigned long pSize, const char *pData)
    : data (pData, pData + pSize)
  {}
}
