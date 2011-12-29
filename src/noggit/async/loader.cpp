#include <noggit/async/loader.h>
#include <noggit/async/loading_thread.h>
#include <noggit/async/object.h>

#include <noggit/Log.h>

namespace noggit
{
  namespace async
  {
    loader::loader (size_t thread_count, QObject* parent)
    {
      Log << tr ("Asynchronous loading with %1 threads.")
             .arg (thread_count)
             .toStdString()
          << std::endl;

      while (thread_count--)
      {
        (new loading_thread (this))->start();
      }
    }
    loader::~loader()
    {
      foreach (loading_thread* thread, findChildren<loading_thread*>())
      {
        thread->quit();
        thread->wait();
      }
    }

    void loader::add_object (object* obj)
    {
      QMutexLocker locker (&_loading_mutex);

      _objects_to_load.append (obj);
    }

    object* loader::next_object_to_load()
    {
      QMutexLocker locker (&_loading_mutex);

      QMutableListIterator<object*> i (_objects_to_load);
      while (i.hasNext())
      {
        object* obj (i.next());
        if (obj->finished_loading())
        {
          i.remove();
        }
        else
        {
          return obj;
        }
      }

      return NULL;
    }
  }
}
