// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/FlattenTool.hpp>

#include <noggit/World.h>
#include <util/qt/overload.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>

namespace noggit
{
  namespace ui
  {
    flatten_blur_tool::flatten_blur_tool(QWidget* parent)
      : QWidget(parent)
      , _radius(10.0f)
      , _speed(2.0f)
      , _angle(45.0f)
      , _orientation(0.0f)
      , _flatten_type(eFlattenType_Linear)
      , _flatten_mode(true, true)
    {
      setMinimumWidth(sizeHint().width());
      auto layout (new QFormLayout (this));

      _type_button_box = new QButtonGroup (this);
      QRadioButton* radio_flat = new QRadioButton ("Flat");
      QRadioButton* radio_linear = new QRadioButton ("Linear");
      QRadioButton* radio_smooth = new QRadioButton ("Smooth");
      QRadioButton* radio_origin = new QRadioButton ("Origin");

      _type_button_box->addButton (radio_flat, (int)eFlattenType_Flat);
      _type_button_box->addButton (radio_linear, (int)eFlattenType_Linear);
      _type_button_box->addButton (radio_smooth, (int)eFlattenType_Smooth);
      _type_button_box->addButton (radio_origin, (int)eFlattenType_Origin);

      radio_linear->toggle();

      QGroupBox* flatten_type_group (new QGroupBox ("Type", this));
      QGridLayout* flatten_type_layout (new QGridLayout (flatten_type_group));
      flatten_type_layout->addWidget (radio_flat, 0, 0);
      flatten_type_layout->addWidget (radio_linear, 0, 1);
      flatten_type_layout->addWidget (radio_smooth, 1, 0);
      flatten_type_layout->addWidget (radio_origin, 1, 1);

      layout->addRow (flatten_type_group);

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.0f, 1000.0f);
      _radius_spin->setDecimals (2);
      _radius_spin->setValue (_radius);

      layout->addRow ("Radius:", _radius_spin);

      _radius_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _radius_slider->setRange (0, 1000);
      _radius_slider->setSliderPosition (_radius);

      layout->addRow (_radius_slider);

      _speed_spin = new QDoubleSpinBox (this);
      _speed_spin->setRange (0.0f, 10.0f);
      _speed_spin->setDecimals (2);
      _speed_spin->setValue (_speed);

      layout->addRow ("Speed:", _speed_spin);

      _speed_slider = new QSlider (Qt::Orientation::Horizontal, this);
      _speed_slider->setRange (0, 10 * 100);
      _speed_slider->setSingleStep (50);
      _speed_slider->setSliderPosition (_speed * 100);

      layout->addRow(_speed_slider);

      QGroupBox* flatten_blur_group = new QGroupBox("Flatten/Blur", this);
      auto flatten_blur_layout = new QGridLayout(flatten_blur_group);

      flatten_blur_layout->addWidget(_lock_up_checkbox = new QCheckBox(this), 0, 0);
      flatten_blur_layout->addWidget(_lock_down_checkbox = new QCheckBox(this), 0, 1);

      _lock_up_checkbox->setChecked(_flatten_mode.raise);
      _lock_up_checkbox->setText("raise");
      _lock_up_checkbox->setToolTip("Raise the terrain when using the tool");
      _lock_down_checkbox->setChecked(_flatten_mode.lower);
      _lock_down_checkbox->setText("lower");
      _lock_down_checkbox->setToolTip("Lower the terrain when using the tool");

      layout->addRow(flatten_blur_group);

      QGroupBox* flatten_only_group = new QGroupBox("Flatten only", this);
      auto flatten_only_layout = new QFormLayout(flatten_only_group);

      _angle_group = new QGroupBox("Angled mode", this);
      _angle_group->setCheckable(true);
      _angle_group->setChecked(false);

      QGridLayout* angle_layout(new QGridLayout(_angle_group));

      angle_layout->addWidget(_orientation_dial = new QDial(this), 0, 0);
      _orientation_dial->setRange(0, 360);
      _orientation_dial->setWrapping(true);
      _orientation_dial->setSliderPosition(_orientation - 90); // to get ingame orientation
      _orientation_dial->setToolTip("Orientation");
      _orientation_dial->setSingleStep(10);

      _angle_slider = new QSlider(this);
      _angle_slider->setRange(0, 89);
      _angle_slider->setSliderPosition(_angle);
      _angle_slider->setToolTip("Angle");
      _angle_slider->setMinimumHeight(80);
      angle_layout->addWidget(_angle_slider, 0, 1);
      
      flatten_only_layout->addRow(_angle_group);

      _lock_group = new QGroupBox("Lock mode", this);
      _lock_group->setCheckable(true);
      _lock_group->setChecked(false);

      QFormLayout* lock_layout(new QFormLayout(_lock_group));

      lock_layout->addRow("X:", _lock_x = new QDoubleSpinBox(this));
      lock_layout->addRow("Z:", _lock_z = new QDoubleSpinBox(this));
      lock_layout->addRow("H:", _lock_h = new QDoubleSpinBox(this));

      _lock_x->setRange(0.0, 34133.0);
      _lock_x->setDecimals(3);
      _lock_z->setRange(0.0, 34133.0);
      _lock_z->setDecimals(3);
      _lock_h->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _lock_h->setDecimals(3);
      _lock_h->setMinimumWidth(30);

      flatten_only_layout->addRow(_lock_group);
      layout->addRow(flatten_only_group);

      connect ( _type_button_box, qOverload<int> (&QButtonGroup::buttonClicked)
              , [&] (int id)
                {
                  _flatten_type = id;
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

      connect( _lock_up_checkbox, &QCheckBox::stateChanged
               , [&] (int state)
                 {
                   _flatten_mode.raise = state;
                 }
             );

      connect( _lock_down_checkbox, &QCheckBox::stateChanged
               , [&] (int state)
                 {
                   _flatten_mode.lower = state;
                 }
             );

      connect ( _angle_slider, &QSlider::valueChanged
                , [&] (int v)
                  {
                    _angle = v;
                  }
                );

      connect ( _orientation_dial, &QDial::valueChanged
                , [this] (int v)
                  {
                    setOrientation(v + 90.0f);
                  }
                );

      connect ( _lock_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _lock_pos.x = v;
                  }
                );

      connect ( _lock_h, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _lock_pos.y = v;
                  }
              );

      connect ( _lock_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
                , [&] (double v)
                  {
                    _lock_pos.z = v;
                  }
              );
    }

    void flatten_blur_tool::flatten (World* world, math::vector_3d const& cursor_pos, float dt)
    {
      world->flattenTerrain ( cursor_pos
                            , 1.f - pow (0.5f, dt *_speed)
                            , _radius
                            , _flatten_type
                            , _flatten_mode
                            , use_ref_pos() ? _lock_pos : cursor_pos
                            , math::degrees (angled_mode() ? _angle : 0.0f)
                            , math::degrees (angled_mode() ? _orientation : 0.0f)
                            );
    }

    void flatten_blur_tool::blur (World* world, math::vector_3d const& cursor_pos, float dt)
    {
      world->blurTerrain ( cursor_pos
                         , 1.f - pow (0.5f, dt * _speed)
                         , _radius
                         , _flatten_type
                         , _flatten_mode
                         );
    }

    void flatten_blur_tool::nextFlattenType()
    {
      _flatten_type = ( ++_flatten_type ) % eFlattenType_Count;
      _type_button_box->button (_flatten_type)->toggle();
    }

    void flatten_blur_tool::nextFlattenMode()
    {
      _flatten_mode.next();

      QSignalBlocker const up_lock(_lock_up_checkbox);
      QSignalBlocker const down_lock(_lock_down_checkbox);
      _lock_up_checkbox->setChecked(_flatten_mode.raise);
      _lock_down_checkbox->setChecked(_flatten_mode.lower);
    }

    void flatten_blur_tool::toggleFlattenAngle()
    {
      _angle_group->setChecked(!angled_mode());
    }

    void flatten_blur_tool::toggleFlattenLock()
    {
      _lock_group->setChecked(!use_ref_pos());
    }

    void flatten_blur_tool::lockPos (math::vector_3d const& cursor_pos)
    {
      _lock_pos = cursor_pos;
      _lock_x->setValue (_lock_pos.x);
      _lock_h->setValue (_lock_pos.y);
      _lock_z->setValue (_lock_pos.z);

      if (!use_ref_pos())
      {
        toggleFlattenLock();
      }
    }

    void flatten_blur_tool::changeRadius(float change)
    {
      _radius_spin->setValue (_radius + change);
    }

    void flatten_blur_tool::changeSpeed(float change)
    {
      _speed_spin->setValue(_speed + change);
    }

    void flatten_blur_tool::setSpeed(float speed)
    {
      _speed_spin->setValue(speed);
    }

    void flatten_blur_tool::changeOrientation(float change)
    {
      setOrientation(_orientation + change);
    }

    void flatten_blur_tool::setOrientation (float orientation)
    {
      QSignalBlocker const blocker (_orientation_dial);

      _orientation = orientation;
      while (_orientation >= 360.0f)
      {
        _orientation -= 360.0f;
      }
      while (_orientation < 0.0f)
      {
        _orientation += 360.0f;
      }
      _orientation_dial->setSliderPosition(_orientation - 90.0f);
    }

    void flatten_blur_tool::changeAngle(float change)
    {
      _angle = std::min(89.0f, std::max(0.0f, _angle + change));
      _angle_slider->setSliderPosition(_angle);
    }

    void flatten_blur_tool::changeHeight(float change)
    {
      _lock_h->setValue(_lock_pos.y + change);
    }

    void flatten_blur_tool::setRadius(float radius)
    {
      _radius_spin->setValue(radius);
    }

    QSize flatten_blur_tool::sizeHint() const
    {
      return QSize(215, height());
    }
  }
}
