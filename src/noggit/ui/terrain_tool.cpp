// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/terrain_tool.hpp>

#include <noggit/Environment.h>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QCheckBox>


namespace ui
{
  terrain_tool::terrain_tool()
    : QWidget(nullptr)
    , _radius(15.0f)
    , _inner_radius(0.0f)
    , _speed(2.0f)
    , _vertex_angle (0.0f)
    , _vertex_orientation (0.0f)
    , _edit_type(Environment::getInstance()->groundBrushType)
    , _vertex_mode(eVertexMode_Center)
  {
    setWindowTitle("Raise / Lower");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    auto layout (new QFormLayout (this));

    _type_button_group = new QButtonGroup (this);
    QRadioButton* radio_flat = new QRadioButton ("Flat");
    QRadioButton* radio_linear = new QRadioButton ("Linear");
    QRadioButton* radio_smooth = new QRadioButton ("Smooth");
    QRadioButton* radio_polynomial = new QRadioButton ("Polynomial");
    QRadioButton* radio_trigo = new QRadioButton ("Trigonom");
    QRadioButton* radio_quadra = new QRadioButton ("Quadratic");
    QRadioButton* radio_gauss = new QRadioButton ("Gaussian");
    QRadioButton* radio_vertex = new QRadioButton ("Vertex");

    _type_button_group->addButton (radio_flat, (int)eTerrainType_Flat);
    _type_button_group->addButton (radio_linear, (int)eTerrainType_Linear);
    _type_button_group->addButton (radio_smooth, (int)eTerrainType_Smooth);
    _type_button_group->addButton (radio_polynomial, (int)eTerrainType_Polynom);
    _type_button_group->addButton (radio_trigo, (int)eTerrainType_Trigo);
    _type_button_group->addButton (radio_quadra, (int)eTerrainType_Quadra);
    _type_button_group->addButton (radio_gauss, (int)eTerrainType_Gaussian);
    _type_button_group->addButton (radio_vertex, (int)eTerrainType_Vertex);

    radio_linear->toggle();

    QGridLayout* terrain_type_layout (new QGridLayout (this));
    terrain_type_layout->addWidget (radio_flat, 0, 0);
    terrain_type_layout->addWidget (radio_linear, 0, 1);
    terrain_type_layout->addWidget (radio_smooth, 1, 0);
    terrain_type_layout->addWidget (radio_polynomial, 1, 1);
    terrain_type_layout->addWidget (radio_trigo, 2, 0);
    terrain_type_layout->addWidget (radio_quadra, 2, 1);
    terrain_type_layout->addWidget (radio_gauss, 3, 0);
    terrain_type_layout->addWidget (radio_vertex, 3, 1);

    QGroupBox* terrain_type_group (new QGroupBox ("Type"));
    terrain_type_group->setLayout (terrain_type_layout);
    layout->addRow (terrain_type_group);

    _radius_spin = new QDoubleSpinBox (this);
    _radius_spin->setRange (0.0f, 1000.0f);
    _radius_spin->setDecimals (2);
    _radius_spin->setValue (_radius);


    layout->addRow (new QLabel("Radius:"));
    layout->addRow ("Outer:", _radius_spin);

    _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _radius_slider->setRange (0, 1000);
    _radius_slider->setSliderPosition ((int)std::round (_radius));
    
    layout->addRow (_radius_slider);

    _inner_radius_spin = new QDoubleSpinBox (this);
    _inner_radius_spin->setRange (0.0f, 1.0f);
    _inner_radius_spin->setDecimals (2);
    _inner_radius_spin->setValue (_inner_radius);

    layout->addRow ("Inner:", _inner_radius_spin);

    _inner_radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _inner_radius_slider->setRange (0, 100);
    _inner_radius_slider->setSliderPosition ((int)std::round (_inner_radius * 100));

    layout->addRow (_inner_radius_slider);

    _speed_spin = new QDoubleSpinBox (this);
    _speed_spin->setRange (0.0f, 10.0f);
    _speed_spin->setDecimals (2);
    _speed_spin->setValue (_speed);

    layout->addRow ("Speed:", _speed_spin);

    _speed_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _speed_slider->setRange (0, 10 * 100);
    _speed_slider->setSingleStep (50);
    _speed_slider->setSliderPosition (_speed * 100);
    
    layout->addRow (_speed_slider);

    _vertex_button_group = new QButtonGroup (this);
    QRadioButton* radio_mouse = new QRadioButton ("Mouse");
    QRadioButton* radio_center = new QRadioButton ("Selection center");

    _vertex_button_group->addButton (radio_mouse, (int)eVertexMode_Mouse);
    _vertex_button_group->addButton (radio_center, (int)eVertexMode_Center);

    radio_mouse->toggle();
    
    QGridLayout* vertex_type_layout (new QGridLayout (this));
    vertex_type_layout->addWidget (radio_mouse, 0, 0);
    vertex_type_layout->addWidget (radio_center, 0, 1);

    QGroupBox* vertex_type_group (new QGroupBox ("Vertex edit relative to"));
    vertex_type_group->setLayout (vertex_type_layout);
    layout->addRow (vertex_type_group);

    connect ( _type_button_group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked)
            , [&] (int id)
              {
                _edit_type = id;
              }
            );

    connect ( _radius_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                _radius = v;
                QSignalBlocker const blocker(_radius_slider);
                _radius_slider->setSliderPosition ((int)std::round (v));
               }
            );

    connect ( _radius_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                _radius = v;
                 QSignalBlocker const blocker(_radius_spin);
                 _radius_spin->setValue(v);
              }
            );

    connect ( _inner_radius_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
            , [&] (double v)
              {
                _inner_radius = v;
                QSignalBlocker const blocker(_inner_radius_slider);
                _inner_radius_slider->setSliderPosition ((int)std::round (v * 100));
               }
            );

    connect ( _inner_radius_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
            , [&] (int v)
              {
                _inner_radius = v / 100.0f;
                 QSignalBlocker const blocker(_inner_radius_spin);
                 _inner_radius_spin->setValue(_inner_radius);
              }
            );

    connect ( _speed_spin, static_cast<void (QDoubleSpinBox::*) (double)> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  _speed = v;
                  QSignalBlocker const blocker(_speed_slider);
                  _speed_slider->setSliderPosition ((int)std::round (v * 100.0f));
                }
              );

    connect ( _speed_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
              , [&] (int v)
                {
                  _speed = v / 100.0f;
                  QSignalBlocker const blocker(_speed_spin);
                  _speed_spin->setValue (_speed);
                }
              );

    connect ( _vertex_button_group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked)
            , [&] (int id)
              {
                _vertex_mode = id;
              }
            );
  }

  void terrain_tool::changeTerrain(math::vector_3d const& pos, float dt)
  {
    if(_edit_type != eTerrainType_Vertex)
    {
      gWorld->changeTerrain(pos, dt*_speed, _radius, _edit_type, _inner_radius);
    }
    else
    {
      // < 0 ==> control is pressed
      if (dt >= 0.0f)
      {
        gWorld->selectVertices(pos, _radius);
      }
      else
      {
        if (gWorld->deselectVertices(pos, _radius))
        {
          _vertex_angle = math::degrees (0.0f);
          _vertex_orientation = math::degrees (0.0f);
          gWorld->clearVertexSelection();
        }
      }      
    }
  }

  void terrain_tool::moveVertices(float dt)
  {
    gWorld->moveVertices(dt * _speed);
  }

  void terrain_tool::flattenVertices()
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      gWorld->flattenVertices (gWorld->vertexCenter().y);
    }
  }

  void terrain_tool::nextType()
  {
    _edit_type = (++_edit_type) % eTerrainType_Count;
    _type_button_group->button (_edit_type)->toggle();
  }

  void terrain_tool::setRadius(float radius)
  {
    _radius_spin->setValue(radius);
  }

  void terrain_tool::changeRadius(float change)
  {
    setRadius (_radius + change);
  }

  void terrain_tool::changeInnerRadius(float change)
  {
    _inner_radius_spin->setValue(_inner_radius + change);
  }

  void terrain_tool::changeSpeed(float change)
  {
    _speed_spin->setValue(_speed + change);
  }

  void terrain_tool::changeOrientation(math::vector_3d const& pos, float change)
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      _vertex_orientation._ += change;

      while (_vertex_orientation._ < 0.0f)
      {
        _vertex_orientation._ += 360.0f;
      }
      while (_vertex_orientation._ > 360.0f)
      {
        _vertex_orientation._ -= 360.0f;
      }
      updateVertices(pos);
    }
  }

  void terrain_tool::setOrientRelativeTo(math::vector_3d const& pos)
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      math::vector_3d const& center = gWorld->vertexCenter();
      _vertex_orientation = math::radians (std::atan2(center.z - pos.z, center.x - pos.x));
      updateVertices(pos);
    }
  }

  void terrain_tool::changeAngle(math::vector_3d const& pos, float change)
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      _vertex_angle = math::degrees (std::max(-89.0f, std::min(89.0f, _vertex_angle._ + change)));
      updateVertices(pos);
    }
  }

  void terrain_tool::updateVertices(math::vector_3d const& cursor_pos)
  {
    gWorld->orientVertices ( _vertex_mode == eVertexMode_Mouse
                           ? cursor_pos
                           : gWorld->vertexCenter()
                           , _vertex_angle
                           , _vertex_orientation
                           );
  }
}

