// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>


namespace ui
{
  class terrain_tool : public QWidget
  {
  public:
    terrain_tool();

    void changeTerrain(math::vector_3d const& pos, float dt);

    void nextType();
    void changeRadius(float change);
    void changeInnerRadius(float change);
    void changeSpeed(float change);

    void setRadius (float radius);
    
    // vertex edit only functions
    void moveVertices(float dt);
    void flattenVertices();

    void changeOrientation(math::vector_3d const& pos, float change);
    void changeAngle(math::vector_3d const& pos, float change);
    void setOrientRelativeTo(math::vector_3d const& pos);

    float brushRadius() const { return _radius; }
    float innerRadius() const { return _inner_radius;  }
  
  private:
    void updateVertices(math::vector_3d const& cursor_pos);

    static const int winWidth = 180;
    
    float _radius;
    float _speed;
    float _inner_radius;
    math::degrees _vertex_angle;
    math::degrees _vertex_orientation;

    bool _tablet;

    int& _edit_type;
    int _vertex_mode;
  
    // UI stuff:

    QButtonGroup* _type_button_group;
    QButtonGroup* _vertex_button_group;

    QSlider* _radius_slider;
    QSlider* _inner_radius_slider;
    QSlider* _speed_slider;
    QDoubleSpinBox* _radius_spin;
    QDoubleSpinBox* _inner_radius_spin;
    QDoubleSpinBox* _speed_spin;
  };
}

