// loading_thread.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#include <noggit/async/loading_thread.h>
#include <noggit/async/loader.h>
#include <noggit/async/object.h>

#include <QTimerEvent>

namespace noggit
{
  namespace async
  {
    loading_thread::loading_thread (loader* async_loader)
      : QThread (async_loader)
      , _async_loader (async_loader)
    {
      startTimer (20);
    }

    void loading_thread::timerEvent (QTimerEvent* event)
    {
      killTimer (event->timerId());
      while (object* obj = _async_loader->next_object_to_load())
      {
        obj->finish_loading();
      }
      startTimer (20);
    }
  }
}
