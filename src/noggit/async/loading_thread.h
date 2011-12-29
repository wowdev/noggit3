#ifndef __NOGGIT_ASYNC_LOADING_THREAD_H
#define __NOGGIT_ASYNC_LOADING_THREAD_H

#include <QThread>

namespace noggit
{
  namespace async
  {
    class loader;

    class loading_thread : public QThread
    {
      Q_OBJECT
    public:
      loading_thread (loader* async_loader);

    protected:
      virtual void timerEvent (QTimerEvent*);

    private:
      loader* _async_loader;
    };
  }
}

#endif
