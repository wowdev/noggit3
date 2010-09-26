#ifndef ASYNCOBJECT_H
#define ASYNCOBJECT_H

class AsyncObject
{
public:
  virtual bool finishedLoading() = 0;
  virtual void finishLoading() = 0;
};

#endif //ASYNCOBJECT_H
