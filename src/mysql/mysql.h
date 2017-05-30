// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cinttypes>
#include <cstddef>

namespace mysql
{
  bool hasMaxUIDStoredDB(std::size_t mapID);
  std::uint32_t getGUIDFromDB(std::size_t mapID);
  void insertUIDinDB(std::size_t mapID, std::uint32_t NewUID);
  void updateUIDinDB (std::size_t mapID, std::uint32_t NewUID);
};
