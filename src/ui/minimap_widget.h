#ifndef __MINIMAPWINDOWUI_H
#define __MINIMAPWINDOWUI_H

#include <QWidget>

class Vec3D;
class World;

namespace ui
{
  //! \todo Make this a fixed square somehow.
  class minimap_widget : public QWidget
  {
  Q_OBJECT

  public:
    minimap_widget (QWidget* parent = NULL);

    virtual QSize sizeHint() const;

    inline const World const* world (World const* world_)
      { _world = world_; update(); return _world; }
    inline const World const* world() const { return _world; }

    inline const bool& draw_skies (const bool& draw_skies_)
      { _draw_skies = draw_skies_; update(); return _draw_skies; }
    inline const bool& draw_skies() const { return _draw_skies; }

    inline const bool& draw_camera (const bool& draw_camera_)
      { _draw_camera = draw_camera_; update(); return _draw_camera; }
    inline const bool& draw_camera() const { return _draw_camera; }

    inline const bool& draw_boundaries (const bool& draw_boundaries_)
      { _draw_boundaries = draw_boundaries_; update(); return _draw_boundaries; }
    inline const bool& draw_boundaries() const { return _draw_boundaries; }

  protected:
    virtual void paintEvent (QPaintEvent*);
    virtual void mouseDoubleClickEvent (QMouseEvent*);

  signals:
    void map_clicked (const Vec3D&);
    void tile_clicked (const QPoint&);

  private:
    World const* _world;
    bool _draw_skies;
    bool _draw_camera;
    bool _draw_boundaries;
  };
}

#endif
