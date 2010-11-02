#ifndef APPSTATE_H
#define APPSTATE_H

#include <SDL/SDL.h>

/*!
	\class AppState
	\brief This class is a base for the two different states of our application, the menu and the 3D view.
	\see Menu, MapView
*/
class AppState
{
public:
	AppState() {};
	virtual ~AppState() {};

	/*!
		\brief This method gets called every tick to do calculations. It will be called right before display().
		\param t The absolute running time.
		\param dt The time difference since the last call.
	*/
	virtual void tick(float t, float dt) { };
	
	/*!
		\brief This method gets called every tick to do display your AppState. It will be called right after tick().
		\param t The absolute running time.
		\param dt The time difference since the last call.
	*/
	virtual void display(float t, float dt) { };

	/*!
		\brief This method gets called upon mouse movement.
		\param e The event given by SDL containing information about the mouse motion.
	*/
	virtual void mousemove(SDL_MouseMotionEvent *e) { };
	
	/*!
	 \brief This method gets called upon a mouse click.
	 \param e The event given by SDL containing information about the mouse click.
	 */
	virtual void mouseclick(SDL_MouseButtonEvent *e) { };
	
	/*!
	 \brief This method gets called when a key is pressed or released.
	 \param e The event given by SDL containing information about the key stroke.
	 */
	virtual void keypressed(SDL_KeyboardEvent *e) { };
	
	/*!
	 \brief This is called when the window is getting resized.
	 */
	virtual void resizewindow() {};
};

#endif
