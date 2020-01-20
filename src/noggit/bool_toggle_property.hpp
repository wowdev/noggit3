// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtCore/QObject>

namespace noggit
{
  struct bool_toggle_property : QObject
  {
  private:
    Q_OBJECT

    bool _value;

  signals:
    void changed (bool);

  public slots:
    void set (bool v)
    {
      if (_value != v)
      {
        _value = v;
        emit changed (v);
      }
    }
    bool get() const
    {
      return _value;
    }
    void toggle()
    {
      set(!_value);
    }
  public:
    bool_toggle_property (bool value)
      : _value (value)
    {}
  };
}
