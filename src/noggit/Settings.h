// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <boost/optional.hpp>

class Settings
{
public:
  static Settings* getInstance();

  bool random_rotation;      // True = Random rotation on model insert.
  bool random_tilt;      // True = Random tileing on model insert.
  bool random_size;      // True = Random sizing on model insert.
  bool copy_autoselect;  //
  bool copyModelStats;

  int FarZ;        // the far clipping value

  bool AutoSelectingMode;  // true activates auto selection when you deselect a model. False not.

  bool tabletMode;

  struct mysql_connection_info
  {
    std::string Server;
    std::string User;
    std::string Pass;
    std::string Database;
  };
  boost::optional<mysql_connection_info> mysql = boost::none;

  const bool& noAntiAliasing() const;
  void readFromDisk();
  bool saveToDisk();

  std::string gamePath;
  std::string projectPath;
  std::string wodSavePath;
  std::string importFile;
  std::string wmvLogFile;

private:
  bool _noAntiAliasing;

  Settings();
  bool createConfigFile();
  static Settings* instance;
};
