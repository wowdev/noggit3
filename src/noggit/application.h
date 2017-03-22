// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncLoader.h>
#include <noggit/DBC.h>
#include <noggit/ui/main_window.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

class Noggit
{
  std::unique_ptr<noggit::ui::main_window> main_window;
public:
  Noggit();

  int start(int argc, char *argv[]);

  bool pop;

private:
  void initPath(char *argv[]);
  void parseArgs(int argc, char *argv[]);
  void loadMPQs();

  boost::filesystem::path wowpath;

  AreaDB areaDB;

public:
  std::unique_ptr<AsyncLoader> asyncLoader;

  bool fullscreen;
  bool doAntiAliasing;

  int xres;
  int yres;
};
