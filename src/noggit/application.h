// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncLoader.h>
#include <noggit/DBC.h>
#include <noggit/FreeType.h> // fonts.
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

  inline AsyncLoader* loader()
  {
    return asyncLoader.get();
  }

  inline const freetype::font_data& getArial12() const
  {
    return arial12;
  }

  inline const freetype::font_data& getArialn13() const
  {
    return arialn13;
  }

  inline const freetype::font_data& getArial14() const
  {
    return arial14;
  }

  inline const freetype::font_data& getArial16() const
  {
    return arial16;
  }

private:
  void initPath(char *argv[]);
public:
  void initFont();
private:
  void parseArgs(int argc, char *argv[]);
  void loadMPQs();

  boost::filesystem::path wowpath;

  AreaDB areaDB;

public:
  std::unique_ptr<AsyncLoader> asyncLoader;

  bool fullscreen;
  bool doAntiAliasing;
private:

  freetype::font_data arialn13, arial12, arial14, arial16;
public:
  int xres;
  int yres;
};

extern Noggit app;
