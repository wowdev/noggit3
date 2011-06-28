#ifndef APPSTATE_H
#define APPSTATE_H

#include <list>
#include <SDL/SDL.h>

class HotKeyReceiver
{
public:
};

/*!
  \class AppState
  \brief This class is a base for the two different states of our application, the menu and the 3D view.
  \see Menu, MapView
*/
class AppState
{
protected:
  typedef void (AppState::* Function)();
  
  enum Modifier
  {
    MOD_shift = 0x01,
    MOD_ctrl  = 0x02,
    MOD_alt   = 0x04,
    MOD_meta  = 0x08,
    MOD_num   = 0x10,
    MOD_caps  = 0x20,
    MOD_mode  = 0x40,
    MOD_none  = 0x00,
  };
  struct HotKey
  {
    SDLKey key;
    size_t modifiers;
    AppState::Function function;
  };
  
  std::list<HotKey> hotkeys;
  
  void addHotkey(SDLKey key, size_t modifiers, AppState::Function function)
  {
    HotKey h = { key, modifiers, function };
    hotkeys.push_back( h );
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

    size_t modifier = ( e->keysym.mod == KMOD_NONE ) ? ( MOD_none ) : (
      ( ( e->keysym.mod & KMOD_SHIFT ) ? MOD_shift : 0 ) |
      ( ( e->keysym.mod & KMOD_CTRL  ) ? MOD_ctrl  : 0 ) |
      ( ( e->keysym.mod & KMOD_ALT   ) ? MOD_alt   : 0 ) |
      ( ( e->keysym.mod & KMOD_META  ) ? MOD_meta  : 0 ) |
      //( ( e->keysym.mod & KMOD_NUM   ) ? MOD_num   : 0 ) |
      ( ( e->keysym.mod & KMOD_CAPS  ) ? MOD_caps  : 0 ) |
      ( ( e->keysym.mod & KMOD_MODE  ) ? MOD_mode  : 0 ) );
    
    //LogError << modifier<< std::endl;

    for( std::list<HotKey>::iterator it = hotkeys.begin(); it != hotkeys.end(); ++it )
    {
      if( e->keysym.sym == it->key && modifier == it->modifiers )
      {
        (this->*(it->function))();
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
  
  /*!
   \brief This method gets called upon a mouse click.
   \param e The event given by SDL containing information about the mouse click.
   */
  virtual void mouseclick(SDL_MouseButtonEvent* /*e*/) { }
  
  /*!
   \brief This method gets called when a key is pressed or released.
   \param e The event given by SDL containing information about the key stroke.
   */
  virtual void keypressed(SDL_KeyboardEvent* /*e*/) { }
  
  /*!
   \brief This is called when the window is getting resized.
   */
  virtual void resizewindow() {}
};

#endif
