// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Settings.h>

#include <cstdint>

namespace mysql
{
  bool hasMaxUIDStoredDB(Settings::mysql_connection_info const& info, std::size_t mapID);
  std::uint32_t getGUIDFromDB(Settings::mysql_connection_info const& info, std::size_t mapID);
  void insertUIDinDB(Settings::mysql_connection_info const& info, std::size_t mapID, std::uint32_t NewUID);
  void updateUIDinDB (Settings::mysql_connection_info const& info, std::size_t mapID, std::uint32_t NewUID);
};
