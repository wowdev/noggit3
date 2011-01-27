#ifndef UIEVENTCLASSES_H
#define UIEVENTCLASSES_H

#include <cassert>

class UIEventListener {
};

class UIEventSender {
public:
	typedef void (UIEventListener::*EventHandlerType)();
protected:
	EventHandlerType eventHandler;
	UIEventListener* listener;
public:
	UIEventSender(EventHandlerType _eventHandler, UIEventListener* _listener) : eventHandler(_eventHandler), listener(_listener) {
		assert( eventHandler && listener );
	}
	UIEventSender() : eventHandler(NULL), listener(NULL) { }
};

#endif //UIEVENTCLASSES_H