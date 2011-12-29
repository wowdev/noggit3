#include <noggit/async/loading_thread.h>
#include <noggit/async/loader.h>
#include <noggit/async/object.h>

namespace noggit
{
  namespace async
  {
    loading_thread::loading_thread (loader* async_loader)
      : QThread (async_loader)
      , _async_loader (async_loader)
    {
      startTimer (5);
    }

    void loading_thread::timerEvent (QTimerEvent*)
    {
      object* obj (_async_loader->next_object_to_load());
      if (obj)
      {
        obj->finish_loading();
      }
    }
  }
}
