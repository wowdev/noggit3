// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/DetailInfos.h>

#include <QtWidgets/QFormLayout>

namespace ui
{
  detail_infos::detail_infos()
    : noggit::ui::widget (nullptr)
  {
    setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);
    auto layout (new QFormLayout (this));

    layout->addRow (_info_text = new QLabel (this));
  }

  void detail_infos::setText (const std::string& t)
  {
    _info_text->setText (t.c_str ());
  }
}
