// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncLoader.h>
#include <noggit/DBC.h>
#include <noggit/FreeType.h> // fonts.

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

class AppState;

#ifdef _WIN32
#include <external/wacom/MSGPACK.H>
#include <external/wacom/WINTAB.h>
#define PACKETDATA  (PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE  PK_BUTTONS
#include <external/wacom/PKTDEF.H>
#include <external/wacom/Utils.h>
HWND static WindowHandle;
HCTX static NEAR TabletInit(HWND hWnd);
#endif

struct SDL_Surface;

class Noggit
{
public:
#ifdef _WIN32
  UINT pressure;
  HCTX hCtx;
  BOOL tabletActive;
#endif
  Noggit();

  int start(int argc, char *argv[]);

  bool pop;

  inline AsyncLoader* loader()
  {
    return asyncLoader.get();
  }

  inline std::vector<AppState*>& getStates()
  {
    return states;
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

  inline const freetype::font_data& getSkurri32() const
  {
    return skurri32;
  }

  inline const freetype::font_data& getFritz16() const
  {
    return fritz16;
  }

private:
  void initPath(char *argv[]);
  void initFont();
  void initEnv();

  void parseArgs(int argc, char *argv[]);
  void loadMPQs();

  unsigned int ticks;
  unsigned int time = 0;
  void mainLoop (SDL_Surface* primary);

  boost::filesystem::path wowpath;

  AreaDB areaDB;
  std::unique_ptr<AsyncLoader> asyncLoader;
  std::vector<AppState*> states;

  bool fullscreen;
  bool doAntiAliasing;

  freetype::font_data arialn13, arial12, arial14, arial16, arial24, arial32, morpheus40, skurri32, fritz16;
public:
  int xres;
  int yres;
};

extern Noggit app;
