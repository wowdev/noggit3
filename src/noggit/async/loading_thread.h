// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

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
