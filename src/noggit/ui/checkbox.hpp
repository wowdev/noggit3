// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/bool_toggle_property.hpp>

#include <QtWidgets/QCheckBox>

namespace noggit
{
  namespace ui
  {
    class checkbox : public QCheckBox
    {
    public:
      checkbox ( QString label
               , bool_toggle_property* prop
               , QWidget* parent = nullptr
               )
        : QCheckBox (label, parent)
      {
        connect ( this, &QCheckBox::toggled
                , prop, &bool_toggle_property::set
                );
        connect ( prop, &bool_toggle_property::changed
                , this, &QCheckBox::setChecked
                );
        setChecked (prop->get());
      }
    };
  }
}
