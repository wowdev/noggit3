// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/texture_swapper.hpp>

#include <math/vector_3d.hpp>
#include <noggit/ui/CurrentTexture.h>
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

      auto texture_display (new noggit::ui::current_texture);
      texture_display->set_texture("tileset\\generic\\black.blp");
      

      QPushButton* select = new QPushButton("Select", this);
      QPushButton* swap_adt = new QPushButton("Swap ADT", this);

      layout->addRow(new QLabel("Texture to swap"));
      layout->addRow(texture_display);
      layout->addRow(select);
      layout->addRow(swap_adt);

      connect(select, &QPushButton::clicked, [this, texture_display]() {
        _texture_to_swap = *selected_texture::get();
        if (_texture_to_swap)
        {
          texture_display->set_texture(_texture_to_swap.get()->filename());
        }
      });

      connect(swap_adt, &QPushButton::clicked, [this, camera_pos, world]() {
        if (_texture_to_swap)
        {
          world->swapTexture (*camera_pos, _texture_to_swap.get());
        }
      });
    }
  }
}
