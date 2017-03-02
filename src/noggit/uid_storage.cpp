// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/uid_storage.hpp>

#include <noggit/Log.h>

#include <boost/filesystem.hpp>


uid_storage::uid_storage()
{
  // create the file if not exists
  if (!boost::filesystem::exists("uid.txt"))
  {
    std::ofstream fs;
    fs.open("uid.txt", std::ios::out);
    fs << "# UID storage file" << std::endl;
    fs << "# map_id,max_id" << std::endl;
    fs.close();
  }
  _uidFile = ConfigFile("uid.txt", ",");
}

uid_storage* uid_storage::instance = nullptr;

uid_storage* uid_storage::getInstance()
{
  if (!instance)
  {
    instance = new uid_storage();
  }
  return instance;
}

inline std::string to_str(std::size_t v)
{
  std::stringstream ss;
  ss << v;
  return ss.str();
}

bool uid_storage::hasMaxUIDStored(std::size_t mapID) const
{
  return _uidFile.keyExists(to_str(mapID));
}

uint32_t uid_storage::getMaxUID(std::size_t mapID) const
{
  if (!hasMaxUIDStored(mapID))
  {
    return 0;
  }

  return _uidFile.read<uint32_t>(to_str(mapID));
}

void uid_storage::saveMaxUID(std::size_t mapID, uint32_t uid)
{
  _uidFile.add<uint32_t>(to_str(mapID), uid);
  save();
}

void uid_storage::save()
{
  // create the file if not exists
  std::ofstream fs;

  if (!boost::filesystem::exists("uid.txt"))
  {
    fs.open("uid.txt", std::ios::out);
  }
  else
  {
    fs.open("uid.txt", std::ios::trunc);
  }

  if (!fs.is_open())
  {
    LogError << "Could not open uid.txt" << std::endl;
    return;
  }

  fs << "# UID storage file" << std::endl;
  fs << "# map_id,max_id" << std::endl;
  fs << _uidFile;

  fs.close();
}
