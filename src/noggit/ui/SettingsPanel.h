// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <qt-color-widgets/color_selector.hpp>

#include <QtCore/QSettings>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

namespace util
{
  struct file_line_edit : public QWidget
  {
    enum mode
    {
      files,
      directories,
    };

    file_line_edit (mode, QString browse_title, QWidget* parent);

    QLineEdit* actual;
  };
}

namespace noggit
{
  namespace ui
  {
    class settings : public QDialog
    {
    private:
      util::file_line_edit* gamePathField;
      util::file_line_edit* projectPathField;
      util::file_line_edit* importPathField;
      util::file_line_edit* wmvLogPathField;
      QDoubleSpinBox* viewDistanceField;
      QDoubleSpinBox* farZField;

      QCheckBox* tabletModeCheck;
      QCheckBox* _vsync_cb;

      QGroupBox* _mysql_box;
      QLineEdit* _mysql_server_field;
      QLineEdit* _mysql_user_field;
      QLineEdit* _mysql_pwd_field;
      QLineEdit* _mysql_db_field;

      QButtonGroup* _wireframe_type_group;
      QDoubleSpinBox* _wireframe_radius;
      QDoubleSpinBox* _wireframe_width;
      color_widgets::ColorSelector* _wireframe_color;
      QCheckBox* _wireframe_rainbow_cb;

      QSettings* _settings;
    public:
      settings();
      void discard_changes();
      void save_changes();
    };
  }
}
