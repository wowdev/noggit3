// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cstdint>

class uid_storage
{
public:
  static bool hasMaxUIDStored(uint32_t mapID);
  static uint32_t getMaxUID (uint32_t mapID);
  static void saveMaxUID(uint32_t mapID, uint32_t uid);
  static void remove_uid_for_map(uint32_t map_id);
};
