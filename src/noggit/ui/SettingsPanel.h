// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
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
      util::file_line_edit* wodPathField;
      util::file_line_edit* projectPathField;
      util::file_line_edit* importPathField;
      util::file_line_edit* wmvLogPathField;
      QDoubleSpinBox* viewDistanceField;
      QDoubleSpinBox* farZField;

      QCheckBox* tabletModeCheck;
      QCheckBox* modelStatsCheck;
      QCheckBox* modelsBoxCheck;
      QCheckBox* randRotCheck;
      QCheckBox* randSizeCheck;
      QCheckBox* randTiltCheck;

    public:
      settings();
      void readInValues();
    };
  }
}
