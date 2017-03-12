// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/terrain_tool.hpp>

#include <noggit/Environment.h>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>

namespace ui
{
  terrain_tool::terrain_tool()
    : QWidget(nullptr)
    , _radius(15.0f)
    , _speed(2.0f)
    , _inner_radius(0.0f)
    , _vertex_angle (0.0f)
    , _vertex_orientation (0.0f)
    , _cursor_pos(nullptr)
    , _edit_type(Environment::getInstance()->groundBrushType)
    , _vertex_mode(eVertexMode_Center)
  {
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

    _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _radius_slider->setRange (0, 1000);
    _radius_slider->setSliderPosition ((int)std::round (_radius));

    _inner_radius_spin = new QDoubleSpinBox (this);
    _inner_radius_spin->setRange (0.0f, 1.0f);
    _inner_radius_spin->setDecimals (2);
    _inner_radius_spin->setValue (_inner_radius);
    _inner_radius_spin->setSingleStep(0.05f);

    _inner_radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _inner_radius_slider->setRange (0, 100);
    _inner_radius_slider->setSliderPosition ((int)std::round (_inner_radius * 100));

    QFormLayout* radius_layout (new QFormLayout (this));
    radius_layout->addRow ("Outer:", _radius_spin);
    radius_layout->addRow (_radius_slider);
    radius_layout->addRow ("Inner:", _inner_radius_spin);
    radius_layout->addRow (_inner_radius_slider);

    QGroupBox* radius_group (new QGroupBox ("Radius"));
    radius_group->setLayout (radius_layout);
    layout->addRow (radius_group);

    _speed_spin = new QDoubleSpinBox (this);
    _speed_spin->setRange (0.0f, 10.0f);
    _speed_spin->setDecimals (2);
    _speed_spin->setValue (_speed);

    _speed_slider = new QSlider (Qt::Orientation::Horizontal, this);
    _speed_slider->setRange (0, 10 * 100);
    _speed_slider->setSingleStep (50);
    _speed_slider->setSliderPosition (_speed * 100);
    
    
    QFormLayout* speed_layout (new QFormLayout (this));
    speed_layout->addRow ("Speed:", _speed_spin);
    speed_layout->addRow (_speed_slider);
    
    _speed_box = new QGroupBox (this);
    _speed_box->setLayout (speed_layout);
    layout->addRow (_speed_box);

    _vertex_button_group = new QButtonGroup (this);
    QRadioButton* radio_mouse = new QRadioButton ("Cursor");
    QRadioButton* radio_center = new QRadioButton ("Selection center");

    radio_mouse->setToolTip ("Orient vertices using the cursor pos as reference");
    radio_center->setToolTip ("Orient vertices using the selection center as reference");

    _vertex_button_group->addButton (radio_mouse, (int)eVertexMode_Mouse);
    _vertex_button_group->addButton (radio_center, (int)eVertexMode_Center);

    radio_center->toggle();
    
    QFormLayout* vertex_layout (new QFormLayout ());

    QGridLayout* vertex_type_layout (new QGridLayout (this));
    vertex_type_layout->addWidget (radio_mouse, 0, 0);
    vertex_type_layout->addWidget (radio_center, 0, 1);
    vertex_layout->addItem (vertex_type_layout);

    QGridLayout* vertex_angle_layout (new QGridLayout (this));
    vertex_angle_layout->addWidget (_orientation_dial = new QDial (this), 0, 0);
    _orientation_dial->setRange(0, 360);
    _orientation_dial->setWrapping(true);
    _orientation_dial->setSliderPosition(_vertex_orientation._ - 90); // to get ingame orientation
    _orientation_dial->setToolTip("Orientation");
    _orientation_dial->setSingleStep(10);    
    
    vertex_angle_layout->addWidget (_angle_slider = new QSlider (this), 0, 1);
    _angle_slider->setRange(-89, 89);
    _angle_slider->setSliderPosition(_vertex_angle._);
    _angle_slider->setToolTip("Angle");

    vertex_layout->addItem (vertex_angle_layout);

    _vertex_type_group = new QGroupBox ("Vertex edit");
    _vertex_type_group->setLayout (vertex_layout);
    _vertex_type_group->hide();
    layout->addRow (_vertex_type_group);

    connect ( _type_button_group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked)
            , [&] (int id)
              {
                _edit_type = id;
                updateVertexGroup();
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

    connect ( _angle_slider, static_cast<void (QSlider::*) (int)> (&QSlider::valueChanged)
              , [&] (int v)
                {
                  setAngle (v);
                }
              );

    connect ( _orientation_dial, static_cast<void (QDial::*) (int)> (&QDial::valueChanged)
              , [this] (int v)
                {
                  setOrientation(v + 90.0f);
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
    updateVertexGroup();
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

  void terrain_tool::changeOrientation(float change)
  {
    setOrientation (_vertex_orientation._ + change);
  }

  void terrain_tool::setOrientation (float orientation)
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      QSignalBlocker const blocker (_orientation_dial);

      while (orientation >= 360.0f)
      {
        orientation -= 360.0f;
      }
      while (orientation < 0.0f)
      {
        orientation += 360.0f;
      }

      _vertex_orientation = math::degrees (orientation);
      _orientation_dial->setSliderPosition (_vertex_orientation._ - 90.0f);

      updateVertices();
    }
  }

  void terrain_tool::setOrientRelativeTo(math::vector_3d const& pos)
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      math::vector_3d const& center = gWorld->vertexCenter();
      _vertex_orientation = math::radians (std::atan2(center.z - pos.z, center.x - pos.x));
      updateVertices();
    }
  }

  void terrain_tool::changeAngle(float change)
  {
    setAngle (_vertex_angle._ + change);
  }

  void terrain_tool::setAngle (float angle)
  {
    if (_edit_type == eTerrainType_Vertex)
    {
      QSignalBlocker const blocker (_angle_slider);
      _vertex_angle = math::degrees (std::max(-89.0f, std::min(89.0f, angle)));
      _angle_slider->setSliderPosition (_vertex_angle._);
      updateVertices();
    }
  }

  void terrain_tool::updateVertices()
  {
    gWorld->orientVertices ( _vertex_mode == eVertexMode_Mouse && !!_cursor_pos
                           ? *_cursor_pos
                           : gWorld->vertexCenter()
                           , _vertex_angle
                           , _vertex_orientation
                           );
  }

  void terrain_tool::updateVertexGroup()
  {
    if (_edit_type != eTerrainType_Vertex)
    {
      _vertex_type_group->hide();
      _speed_box->show();
    }
    else
    {
      _vertex_type_group->show();
      _speed_box->hide();
    }
    adjustSize();
  }
}

