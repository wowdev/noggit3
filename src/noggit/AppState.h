// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <SDL.h>

#include <forward_list>
#include <functional>

/*!
\class AppState
\brief This class is a base for the two different states of our application, the menu and the 3D view.
\see Menu, MapView
*/
class AppState
{
protected:
  enum Modifier
  {
    MOD_shift = 0x01,
    MOD_ctrl = 0x02,
    MOD_alt = 0x04,
    MOD_meta = 0x08,
    MOD_num = 0x10,
    MOD_caps = 0x20,
    MOD_mode = 0x40,
    MOD_none = 0x00,
  };
  struct HotKey
  {
    SDLKey key;
    size_t modifiers;
    std::function<void()> function;
    std::function<bool()> condition;
    HotKey (SDLKey k, size_t m, std::function<void()> f, std::function<bool()> c)
      : key (k), modifiers (m), function (f), condition (c) {}
  };

  std::forward_list<HotKey> hotkeys;

  void addHotkey(SDLKey key, size_t modifiers, std::function<void()> function, std::function<bool()> condition = [] { return true; })
  {
    hotkeys.emplace_front (key, modifiers, function, condition);
  }

  bool handleHotkeys(SDL_KeyboardEvent* e)
  {
    /*
    if( mod & KMOD_NUM ) LogError << "NUMLOCK " << std::endl;
    if( mod & KMOD_CAPS ) LogError << "CAPSLOCK " << std::endl;
    if( mod & KMOD_MODE ) LogError << "MODE " << std::endl;
    if( mod & KMOD_LCTRL ) LogError << "LCTRL " << std::endl;
    if( mod & KMOD_RCTRL ) LogError << "RCTRL " << std::endl;
    if( mod & KMOD_LSHIFT ) LogError << "LSHIFT " << std::endl;
    if( mod & KMOD_RSHIFT ) LogError << "RSHIFT " << std::endl;
    if( mod & KMOD_LALT ) LogError << "LALT " << std::endl;
    if( mod & KMOD_RALT ) LogError << "RALT " << std::endl;
    if( mod & KMOD_LMETA ) LogError << "LMETA " << std::endl;
    if( mod & KMOD_RMETA )LogError << "RMETA " << std::endl;
    */
    size_t modifier = (e->keysym.mod == KMOD_NONE) ? (MOD_none) : (
      ((e->keysym.mod & KMOD_SHIFT) ? MOD_shift : 0) |
      ((e->keysym.mod & KMOD_CTRL) ? MOD_ctrl : 0) |
      ((e->keysym.mod & KMOD_ALT) ? MOD_alt : 0) |
      ((e->keysym.mod & KMOD_META) ? MOD_meta : 0) |
      //( ( e->keysym.mod & KMOD_NUM   ) ? MOD_num   : 0 ) |
      ((e->keysym.mod & KMOD_CAPS) ? MOD_caps : 0) |
      ((e->keysym.mod & KMOD_MODE) ? MOD_mode : 0));

    //LogError << modifier<< std::endl;

    for (auto&& hotkey : hotkeys)
    {
      if (e->keysym.sym == hotkey.key && modifier == hotkey.modifiers && hotkey.condition())
      {
        hotkey.function();
        return true;
      }
    }
    return false;
  }

public:
  AppState() {}
  virtual ~AppState() {}

  /*!
  \brief This method gets called every tick to do calculations. It will be called right before display().
  \param t The absolute running time.
  \param dt The time difference since the last call.
  */
  virtual void tick(float /*t*/, float /*dt*/) { }

  /*!
  \brief This method gets called every tick to do display your AppState. It will be called right after tick().
  \param t The absolute running time.
  \param dt The time difference since the last call.
  */
  virtual void display(float /*t*/, float /*dt*/) { }

  /*!
  \brief This method gets called upon mouse movement.
  \param e The event given by SDL containing information about the mouse motion.
  */
  virtual void mousemove(SDL_MouseMotionEvent* /*e*/) { }

  virtual void mouseReleaseEvent (SDL_MouseButtonEvent*)
  {
  }
  virtual void mousePressEvent (SDL_MouseButtonEvent*)
  {
  }

  virtual void keyPressEvent (SDL_KeyboardEvent* e)
  {
    handleHotkeys(e);
  }
  virtual void keyReleaseEvent (SDL_KeyboardEvent*)
  {
  }

  /*!
  \brief This is called when the window is getting resized.
  */
  virtual void resizewindow() {}
};
