#ifndef CURSORSWITCHER_H
#define CURSORSWITCHER_H

#include <QWidget>

//! \todo Namespace and class rename as well as file moving.
class UICursorSwitcher : public QWidget
{
  Q_OBJECT
public:
  UICursorSwitcher (QWidget* parent = NULL);

public slots:
  void set_cursor_type (int value);
  void set_red_color (int value);
  void set_green_color (int value);
  void set_blue_color (int value);
  void set_alpha (int value);
};

#endif
