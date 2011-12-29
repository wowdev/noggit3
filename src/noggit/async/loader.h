#ifndef __NOGGIT_ASYNC_LOADER_H
#define __NOGGIT_ASYNC_LOADER_H

#include <QObject>
#include <QLinkedList>
#include <QMutex>
#include <QSet>

namespace noggit
{
  namespace async
  {
    class object;

    class loader : public QObject
    {
    public:
      loader (size_t thread_count, QObject* parent = NULL);
      ~loader();

      void add_object (object*);

    private:
      object* next_object_to_load();
      QList<object*> _objects_to_load;
      QMutex _loading_mutex;

      friend class loading_thread;
    };
  }
}

#endif
