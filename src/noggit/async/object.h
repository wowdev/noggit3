#ifndef __NOGGIT_ASYNC_OBJECT_H
#define __NOGGIT_ASYNC_OBJECT_H

namespace noggit
{
  namespace async
  {
    class object
    {
    public:
      bool finished_loading() const
      {
        return _finished;
      }
      virtual void finish_loading() = 0;

    protected:
      bool _finished;
    };
  }
}

#endif
