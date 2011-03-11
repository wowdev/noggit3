#ifndef UIEVENTCLASSES_H
#define UIEVENTCLASSES_H

#include <cassert>

/*!

  Your handling class extends UIEventListener. When constructing the button or whatever, you pass UIEventConstructorArgument(SenderClass,ListenerObject,ListenerMethod).
  class MyClass : public UIEventListener {
    MyClass() {
      SenderClass* sc = new SenderClass( UIEventConstructorArgument(SenderClass, this, MyClass::MyHandler) );
    }
    void MyHandler(int i) {
      LogDebug << i << "\n";
    }
  };

 */

class UIEventListener {
};

#define UIEventEventHandlerDefinition(...) typedef void (UIEventListener::*EventHandlerType)(__VA_ARGS__)
#define UIEventClassConstructorArguments EventHandlerType _eventHandler, UIEventListener* _listener
#define UIEventClassConstructorSuperCall() UIEventSender(reinterpret_cast<UIEventSender::EventHandlerType>(_eventHandler), _listener)
#define UIEventEventHandlerCall(...) { EventHandlerType eventHandlerCasted = reinterpret_cast<EventHandlerType>(eventHandler); (listener->*eventHandlerCasted)(__VA_ARGS__); }
#define UIEventConstructorArgument(SenderClass,ListenerObject,ListenerMethod) static_cast<SenderClass::EventHandlerType>(&ListenerMethod), static_cast<UIEventListener*>(ListenerObject)

/*! 

  Your event sending class extends UIEventSender. In your constructor, you take UIEventClassConstructorArguments. Then in the initializer list, you call UIEventClassConstructorSuperCall(). In the event sending method, you call UIEventHandlerCall(...) with your arguments. You can define a new handler structure by also defining UIEventEventHandlerDefinition(...); in your class where ... are the parameter types.
  
  class SenderClass : public UIEventSender {
  public:
	  UIEventEventHandlerDefinition(int);
	  SenderClass(UIEventClassConstructorArguments) : UIEventClassConstructorSuperCall() {
	    UIEventEventHandlerCall(1);
	  }
  };

 */

class UIEventSender {
public:
	UIEventEventHandlerDefinition();
protected:
	EventHandlerType eventHandler;
	UIEventListener* listener;
public:
	UIEventSender(UIEventClassConstructorArguments) : eventHandler(_eventHandler), listener(_listener) {
		assert( eventHandler && listener );
	}
	//UIEventSender() : eventHandler(NULL), listener(NULL) { }
};

#endif //UIEVENTCLASSES_H