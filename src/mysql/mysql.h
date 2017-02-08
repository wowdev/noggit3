// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Settings.h>

#include <cstdint>

namespace mysql
{
  std::uint32_t getGUIDFromDB (Settings::mysql_connection_info const& info);
  void UpdateUIDInDB (Settings::mysql_connection_info const& info, std::uint32_t NewUID);
};
