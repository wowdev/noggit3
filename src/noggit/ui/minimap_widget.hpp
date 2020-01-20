// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QWidget>

namespace math
{
  struct vector_3d;
}
class World;

//! \todo add adt coordinates/name on mouseover
namespace noggit
{
  class camera;

  namespace ui
  {
    //! \todo Make this a fixed square somehow.
    class minimap_widget : public QWidget
    {
      Q_OBJECT

    public:
      minimap_widget (QWidget* parent = nullptr);

      virtual QSize sizeHint() const override;

      inline const World* world (World* const world_)
        { _world = world_; update(); return _world; }
      inline const World* world() const { return _world; }

      inline const bool& draw_skies (const bool& draw_skies_)
        { _draw_skies = draw_skies_; update(); return _draw_skies; }
      inline const bool& draw_skies() const { return _draw_skies; }

      inline const bool& draw_boundaries (const bool& draw_boundaries_)
        { _draw_boundaries = draw_boundaries_; update(); return _draw_boundaries; }
      inline const bool& draw_boundaries() const { return _draw_boundaries; }

      inline void camera (noggit::camera* camera) { _camera = camera; }

    protected:
      virtual void paintEvent (QPaintEvent*) override;
      virtual void mouseDoubleClickEvent (QMouseEvent*) override;
      virtual void mouseMoveEvent(QMouseEvent*) override;

    signals:
      void map_clicked (const ::math::vector_3d&);
      void tile_clicked (const QPoint&);

    private:
      World const* _world;
      noggit::camera* _camera;
      bool _draw_skies;
      bool _draw_camera;
      bool _draw_boundaries;
    };
  }
}
