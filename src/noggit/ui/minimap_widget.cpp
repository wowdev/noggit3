// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/minimap_widget.hpp>

#include <QPaintEvent>
#include <QPainter>
#include <QToolTip>

#include <noggit/Sky.h>
#include <noggit/World.h>
#include <noggit/camera.hpp>

namespace noggit
{
  namespace ui
  {
    minimap_widget::minimap_widget (QWidget* parent)
      : QWidget (parent)
      , _world (nullptr)
      , _camera (nullptr)
      , _draw_skies (false)
    {
      setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
      setMouseTracking(true);
    }

    QSize minimap_widget::sizeHint() const
    {
      return QSize (700, 700);
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
        painter.drawImage (drawing_rect, world()->horizon._qt_minimap);

        if (draw_boundaries())
        {
          //! \todo Draw non-existing tiles aswell?
          painter.setBrush (QColor (255, 255, 255, 30));
          for (size_t i (0); i < 64; ++i)
          {
            for (size_t j (0); j < 64; ++j)
            {
              tile_index const tile (i, j);
              bool changed = false;

              if (world()->mapIndex.hasTile (tile))
              {
                if (world()->mapIndex.tileLoaded (tile))
                {
                  if (world()->mapIndex.has_unsaved_changes(tile))
                  {
                    changed = true;
                  }

                  painter.setPen(QColor::fromRgbF(0.f, 0.f, 0.f, 0.6f));
                }
                else if (world()->mapIndex.isTileExternal(tile))
                {
                  painter.setPen(QColor::fromRgbF(1.0f, 0.7f, 0.5f, 0.6f));
                }
                else
                {
                  painter.setPen (QColor::fromRgbF (0.8f, 0.8f, 0.8f, 0.4f));
                }
              }
              else
              {
                painter.setPen (QColor::fromRgbF (1.0f, 1.0f, 1.0f, 0.05f));
              }

              painter.drawRect ( QRect ( tile_size * i
                                       , tile_size * j
                                       , tile_size
                                       , tile_size
                                       )
                               );

              if (changed)
              {
                painter.setPen(QColor::fromRgbF(1.0f, 1.0f, 0.0f, 1.f));
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
          for (auto const& sky : world()->skies->skies)
          {
            //! \todo Get actual color from sky.
            //! \todo Get actual radius.
            //! \todo Inner and outer radius?
            painter.setPen (Qt::blue);

            painter.drawEllipse ( QPointF ( sky.pos.x * scale_factor
                                          , sky.pos.z * scale_factor
                                          )
                                , 10.0 // radius
                                , 10.0
                                );
          }
        }

        if (_camera)
        {
          painter.setPen (Qt::red);

          QLineF camera_vector ( QPointF ( _camera->position.x * scale_factor
                                         , _camera->position.z * scale_factor
                                         )
                               , QPointF ( _camera->position.x * scale_factor
                                         , _camera->position.z * scale_factor
                                         )
                               + QPointF ( math::cos (_camera->yaw()) * scale_factor
                                         , -math::sin (_camera->yaw()) * scale_factor
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

      if (!world()->mapIndex.hasTile (tile_index (tile.x(), tile.y())))
      {
        event->ignore();
        return;
      }

      event->accept();

      emit map_clicked ( ::math::vector_3d ( (event->pos().x() / float (tile_size)) * TILESIZE
                                           , 0.0f
                                           , (event->pos().y() / float (tile_size)) * TILESIZE
                                           )
                       );
    }

    void minimap_widget::mouseMoveEvent(QMouseEvent* event)
    {
      if (world())
      {
        const int smaller_side((qMin(rect().width(), rect().height()) / 64) * 64);
        const int tile_size(smaller_side / 64);
        int x = event->pos().x(), y = event->pos().y();

        std::string str("ADT: " + std::to_string(x / tile_size) + "_" + std::to_string(y / tile_size));

        QToolTip::showText(mapToGlobal(QPoint(x, y+5)), QString::fromStdString(str));
      }
    }
  }
}
