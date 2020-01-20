// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/terrain_tool.hpp>

#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

namespace noggit
{
  namespace ui
  {
    terrain_tool::terrain_tool(QWidget* parent)
      : QWidget(parent)
      , _edit_type (eTerrainType_Linear)
      , _radius(15.0f)
      , _speed(2.0f)
      , _inner_radius(0.0f)
      , _vertex_angle (0.0f)
      , _vertex_orientation (0.0f)
      , _cursor_pos(nullptr)
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

      QGroupBox* terrain_type_group (new QGroupBox ("Type"));
      QGridLayout* terrain_type_layout (new QGridLayout (terrain_type_group));
      terrain_type_layout->addWidget (radio_flat, 0, 0);
      terrain_type_layout->addWidget (radio_linear, 0, 1);
      terrain_type_layout->addWidget (radio_smooth, 1, 0);
      terrain_type_layout->addWidget (radio_polynomial, 1, 1);
      terrain_type_layout->addWidget (radio_trigo, 2, 0);
      terrain_type_layout->addWidget (radio_quadra, 2, 1);
      terrain_type_layout->addWidget (radio_gauss, 3, 0);
      terrain_type_layout->addWidget (radio_vertex, 3, 1);

      layout->addWidget (terrain_type_group);

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

      QGroupBox* radius_group (new QGroupBox ("Radius"));
      QFormLayout* radius_layout (new QFormLayout (radius_group));
      radius_layout->addRow ("Outer:", _radius_spin);
      radius_layout->addRow (_radius_slider);
      radius_layout->addRow ("Inner:", _inner_radius_spin);
      radius_layout->addRow (_inner_radius_slider);

      layout->addWidget (radius_group);

      _speed_spin = new QDoubleSpinBox (this);
      _speed_spin->setRange (0.0f, 10.0f);
      _speed_spin->setDecimals (2);
      _speed_spin->setValue (_speed);

      _speed_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _speed_slider->setRange (0, 10 * 100);
      _speed_slider->setSingleStep (50);
      _speed_slider->setSliderPosition (_speed * 100);


      _speed_box = new QGroupBox (this);
      QFormLayout* speed_layout (new QFormLayout (_speed_box));
      speed_layout->addRow ("Speed:", _speed_spin);
      speed_layout->addRow (_speed_slider);

      layout->addWidget (_speed_box);

      _vertex_type_group = new QGroupBox ("Vertex edit");
      QVBoxLayout* vertex_layout (new QVBoxLayout (_vertex_type_group));

      _vertex_button_group = new QButtonGroup (this);
      QRadioButton* radio_mouse = new QRadioButton ("Cursor", _vertex_type_group);
      QRadioButton* radio_center = new QRadioButton ("Selection center", _vertex_type_group);

      radio_mouse->setToolTip ("Orient vertices using the cursor pos as reference");
      radio_center->setToolTip ("Orient vertices using the selection center as reference");

      _vertex_button_group->addButton (radio_mouse, (int)eVertexMode_Mouse);
      _vertex_button_group->addButton (radio_center, (int)eVertexMode_Center);

      radio_center->toggle();

      QHBoxLayout* vertex_type_layout (new QHBoxLayout);
      vertex_type_layout->addWidget (radio_mouse);
      vertex_type_layout->addWidget (radio_center);
      vertex_layout->addItem (vertex_type_layout);

      QHBoxLayout* vertex_angle_layout (new QHBoxLayout);
      vertex_angle_layout->addWidget (_orientation_dial = new QDial (_vertex_type_group));
      _orientation_dial->setRange(0, 360);
      _orientation_dial->setWrapping(true);
      _orientation_dial->setSliderPosition(_vertex_orientation._ - 90); // to get ingame orientation
      _orientation_dial->setToolTip("Orientation");
      _orientation_dial->setSingleStep(10);

      vertex_angle_layout->addWidget (_angle_slider = new QSlider (_vertex_type_group));
      _angle_slider->setRange(-89, 89);
      _angle_slider->setSliderPosition(_vertex_angle._);
      _angle_slider->setToolTip("Angle");

      vertex_layout->addItem (vertex_angle_layout);

      layout->addWidget (_vertex_type_group);
      _vertex_type_group->hide();

      connect ( _type_button_group, qOverload<int> (&QButtonGroup::buttonClicked)
              , [&] (int id)
                {
                  _edit_type = static_cast<eTerrainType> (id);
                  updateVertexGroup();
                }
              );

      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  _radius = v;
                  QSignalBlocker const blocker(_radius_slider);
                  _radius_slider->setSliderPosition ((int)std::round (v));
                 }
              );

      connect ( _radius_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  _radius = v;
                   QSignalBlocker const blocker(_radius_spin);
                   _radius_spin->setValue(v);
                }
              );

      connect ( _inner_radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  _inner_radius = v;
                  QSignalBlocker const blocker(_inner_radius_slider);
                  _inner_radius_slider->setSliderPosition ((int)std::round (v * 100));
                 }
              );

      connect ( _inner_radius_slider, &QSlider::valueChanged
              , [&] (int v)
                {
                  _inner_radius = v / 100.0f;
                   QSignalBlocker const blocker(_inner_radius_spin);
                   _inner_radius_spin->setValue(_inner_radius);
                }
              );

      connect ( _speed_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _speed = v;
                    QSignalBlocker const blocker(_speed_slider);
                    _speed_slider->setSliderPosition ((int)std::round (v * 100.0f));
                  }
                );

      connect ( _speed_slider, &QSlider::valueChanged
                , [&] (int v)
                  {
                    _speed = v / 100.0f;
                    QSignalBlocker const blocker(_speed_spin);
                    _speed_spin->setValue (_speed);
                  }
                );

      connect ( _vertex_button_group, qOverload<int> (&QButtonGroup::buttonClicked)
              , [&] (int id)
                {
                  _vertex_mode = id;
                }
              );

      connect ( _angle_slider, &QSlider::valueChanged
              , [this] (int v)
                  {
                    setAngle (v);
                  }
                );

      connect ( _orientation_dial, &QDial::valueChanged
              , [this] (int v)
                  {
                    setOrientation (v + 90.0f);
                  }
                );

      setMinimumWidth(sizeHint().width());
    }

    void terrain_tool::changeTerrain
      (World* world, math::vector_3d const& pos, float dt)
    {
      if(_edit_type != eTerrainType_Vertex)
      {
        world->changeTerrain(pos, dt*_speed, _radius, _edit_type, _inner_radius);
      }
      else
      {
        // < 0 ==> control is pressed
        if (dt >= 0.0f)
        {
          world->selectVertices(pos, _radius);
        }
        else
        {
          if (world->deselectVertices(pos, _radius))
          {
            _vertex_angle = math::degrees (0.0f);
            _vertex_orientation = math::degrees (0.0f);
            world->clearVertexSelection();
          }
        }
      }
    }

    void terrain_tool::moveVertices (World* world, float dt)
    {
      world->moveVertices(dt * _speed);
    }

    void terrain_tool::flattenVertices (World* world)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        world->flattenVertices (world->vertexCenter().y);
      }
    }

    void terrain_tool::nextType()
    {
      _edit_type = static_cast<eTerrainType> ((static_cast<int> (_edit_type) + 1) % eTerrainType_Count);
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

    void terrain_tool::setSpeed(float speed)
    {
      _speed_spin->setValue(speed);
    }

    void terrain_tool::changeOrientation (float change)
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

        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void terrain_tool::setOrientRelativeTo (World* world, math::vector_3d const& pos)
    {
      if (_edit_type == eTerrainType_Vertex)
      {
        math::vector_3d const& center = world->vertexCenter();
        _vertex_orientation = math::radians (std::atan2(center.z - pos.z, center.x - pos.x));
        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
    }

    void terrain_tool::changeAngle (float change)
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
        emit updateVertices(_vertex_mode, _vertex_angle, _vertex_orientation);
      }
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
    }

    QSize terrain_tool::sizeHint() const
    {
      return QSize(215, height());
    }
  }
}
