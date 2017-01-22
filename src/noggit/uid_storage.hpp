// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ConfigFile.h>

class uid_storage
{
public:
  static uid_storage* getInstance();
  
  bool hasMaxUIDStored(std::size_t mapID) const;
  uint32_t getMaxUID(std::size_t mapID) const;

  void saveMaxUID(std::size_t mapID, uint32_t uid);

  // save the uids on the disc
  void save();
  
private:
  ConfigFile _uidFile;

	uid_storage();
	static uid_storage* instance;
};
