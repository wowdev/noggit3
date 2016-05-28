// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

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

      virtual ~object() = default;

    protected:
      bool _finished;
    };
  }
}
