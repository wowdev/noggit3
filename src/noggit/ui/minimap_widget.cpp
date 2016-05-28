// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/minimap_widget.h>

#include <QPaintEvent>
#include <QPainter>

#include <noggit/Sky.h>
#include <noggit/World.h>

namespace noggit
{
  namespace ui
  {
    minimap_widget::minimap_widget (QWidget* parent)
    : QWidget (parent)
    , _world (nullptr)
    , _draw_skies (false)
    , _draw_camera (false)
    {
      setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }

    QSize minimap_widget::sizeHint() const
    {
      return QSize (512, 512);
    }

    //! \todo Only redraw stuff as told in event.
    void minimap_widget::paintEvent (QPaintEvent*)
    {
      //! \note Only take multiples of 1.0 pixels per tile.
      const int smaller_side ((qMin (rect().width(), rect().height()) / 64) * 64);
      const QRect drawing_rect (0, 0, smaller_side, smaller_side);

      const int tile_size (smaller_side / 64);
      const qreal scale_factor (tile_size / TILESIZE);

      QPainter painter (this);
      painter.setRenderHints ( QPainter::Antialiasing
                             | QPainter::TextAntialiasing
                             | QPainter::SmoothPixmapTransform
                             );

      if (world())
      {
        painter.drawImage (drawing_rect, world()->minimap());

        if (draw_boundaries())
        {
          painter.setPen (QColor (255, 255, 0));

          for (size_t i (0); i < 64; ++i)
          {
            for (size_t j (0); j < 64; ++j)
            {
              //! \todo Check, if correct order!
              if (world()->getChanged (j, i))
              {
                painter.drawLine ( tile_size * i
                                 , tile_size * j
                                 , tile_size * (i + 1) - 1
                                 , tile_size * j
                                 );
                painter.drawLine ( tile_size * i
                                 , tile_size * j
                                 , tile_size * i
                                 , tile_size * (j + 1) - 1
                                 );
              }
            }
          }

          //! \todo Draw non-existing tiles aswell?
          painter.setPen (QColor (0, 0, 0, 0));
          painter.setBrush (QColor (255, 255, 255, 30));
          for (size_t i (0); i < 64; ++i)
          {
            for (size_t j (0); j < 64; ++j)
            {
              if (world()->hasTile (j, i))
              {
                painter.drawRect ( QRect ( tile_size * i + 1
                                         , tile_size * j + 1
                                         , tile_size - 2
                                         , tile_size - 2
                                         )
                                 );
              }
            }
          }
        }

        if (draw_skies() && world()->skies)
        {
          foreach (Sky sky, world()->skies->skies)
          {
            //! \todo Get actual color from sky.
            //! \todo Get actual radius.
            //! \todo Inner and outer radius?
            painter.setPen (Qt::blue);

            painter.drawEllipse ( QPointF ( sky.pos.x() * scale_factor
                                          , sky.pos.z() * scale_factor
                                          )
                                , 10.0 // radius
                                , 10.0
                                );
          }
        }

        if (draw_camera())
        {
          painter.setPen (Qt::red);

          QLineF camera_vector ( QPointF ( world()->camera.x() * scale_factor
                                         , world()->camera.z() * scale_factor
                                         )
                               , QPointF ( world()->lookat.x() * scale_factor
                                         , world()->lookat.z() * scale_factor
                                         )
                               );
          camera_vector.setLength (15.0);

          painter.drawLine (camera_vector);
        }
      }
      else
      {
        //! \todo Draw something so user realizes this will become the minimap.
        painter.setPen (Qt::black);
        painter.setFont (QFont ("Arial", 30));
        painter.drawText ( drawing_rect
                         , Qt::AlignCenter
                         , tr ("Select a map on the left side.")
                         );
      }
    }

    void minimap_widget::mouseDoubleClickEvent (QMouseEvent* event)
    {
      if (event->button() != Qt::LeftButton)
      {
        event->ignore();
        return;
      }

      const int smaller_side ((qMin (rect().width(), rect().height()) / 64) * 64);
      const int tile_size (smaller_side / 64);
      //! \note event->pos() / tile_size seems to be using floating point arithmetic, therefore getting wrong results.
      const QPoint tile ( event->pos().x() / tile_size
                        , event->pos().y() / tile_size
                        );

      emit tile_clicked (tile);

      if (!world()->hasTile (tile.y(), tile.x()))
      {
        event->ignore();
        return;
      }

      event->accept();

      emit map_clicked (world(), ::math::vector_3d (tile.x() * TILESIZE, 0.0f, tile.y() * TILESIZE));
    }
  }
}
