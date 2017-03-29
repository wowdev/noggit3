// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtCore/QObject>

namespace noggit
{
  struct unsigned_int_property : QObject
  {
  private:
    Q_OBJECT

    unsigned int _value;

  signals:
    void changed (unsigned int);

  public slots:
    void set (unsigned int v)
    {
      if (_value != v)
      {
        _value = v;
        emit changed (v);
      }
    }
    unsigned int get() const
    {
      return _value;
    }

  public:
    unsigned_int_property (unsigned int value)
      : _value (value)
    {}
  };
}
