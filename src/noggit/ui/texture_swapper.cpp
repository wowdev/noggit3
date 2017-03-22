// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texture_swapper.hpp>

#include <math/vector_3d.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/World.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace noggit
{
  namespace ui
  {
    texture_swapper::texture_swapper ( QWidget* parent
                                     , const math::vector_3d* camera_pos
                                     , World* world
                                     )
      : QWidget (parent)
      , _texture_to_swap()
    {
      setWindowTitle ("Swap");
      setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);

      auto layout (new QFormLayout (this));

      QLabel* texture_display = new QLabel (this);
      texture_display->setMinimumSize(64, 64);
      texture_display->setPixmap (render_blp_to_pixmap ("tileset\\generic\\black.blp", 64, 64));

      QPushButton* select = new QPushButton("Select", this);
      QPushButton* swap_adt = new QPushButton("Swap ADT", this);

      layout->addRow(texture_display);
      layout->addRow(select);
      layout->addRow(swap_adt);

      connect(select, &QPushButton::clicked, [this, texture_display]() {
        _texture_to_swap = *selected_texture::get();
        if (_texture_to_swap)
        {
          texture_display->setPixmap (render_blp_to_pixmap (_texture_to_swap.get()->filename().c_str()));
        }
      });

      connect(swap_adt, &QPushButton::clicked, [this, camera_pos, world]() {
        if (_texture_to_swap)
        {
          world->swapTexture (*camera_pos, _texture_to_swap.get());
        }
      });
    }

    void texture_swapper::closeEvent(QCloseEvent*)
    {
      auto parent (parentWidget());
      if (parent)
      {
        parent->show();
      }
    }
  }
}
