#ifndef ASYNCOBJECT_H
#define ASYNCOBJECT_H

class AsyncObject
{
protected:
	bool finished;
public:
  virtual ~AsyncObject() {}

	virtual bool finishedLoading() const
	{
		return finished;
	}
	virtual void finishLoading() = 0;
};

#endif //ASYNCOBJECT_H
