// signal_blocker.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>

#pragma once

#include <QObject>

namespace helper
{
  namespace qt
  {
    class signal_blocker
    {
    public:
      signal_blocker (QObject* object)
        : _object (object)
        , _old_state (_object->blockSignals (true))
      { }

      signal_blocker (QObject& object)
        : _object (&object)
        , _old_state (_object->blockSignals (true))
      { }

      ~signal_blocker()
      {
        _object->blockSignals (_old_state);
      }

    private:
      QObject* _object;
      bool _old_state;
    };
  }
}
