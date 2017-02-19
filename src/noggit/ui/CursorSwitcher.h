// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/vector_4d.hpp>

#include <QtWidgets/QWidget>

class UICursorSwitcher : public QWidget
{
public:
  UICursorSwitcher(math::vector_4d& color, int& cursor_type);
};
