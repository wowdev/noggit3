// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/uid_storage.hpp>

#include <boost/filesystem.hpp>

#include <QTCore/QSettings>


bool uid_storage::hasMaxUIDStored(uint32_t mapID)
{
  QSettings settings;
  return settings.value ("project/uids/" + QString(mapID), -1).toUInt () != -1;
}

uint32_t uid_storage::getMaxUID(uint32_t mapID)
{
  QSettings settings;
  return settings.value ("project/uids/" + QString(mapID), 0).toUInt();
}

void uid_storage::saveMaxUID(uint32_t mapID, uint32_t uid)
{
  QSettings settings;
  settings.setValue ("project/uids/" + QString(mapID), uid);
}
