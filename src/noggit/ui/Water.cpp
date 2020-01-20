// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/ui/pushbutton.hpp>
#include <noggit/ui/Water.h>
#include <util/qt/overload.hpp>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QRadioButton>

namespace noggit
{
  namespace ui
  {
    water::water ( unsigned_int_property* current_layer
                 , bool_toggle_property* display_all_layers
                 , QWidget* parent
                 )
      : QWidget (parent)
      , _liquid_id(5)
      , _radius(10.0f)
      , _angle(10.0f)
      , _orientation(0.0f)
      , _locked(false)
      , _angled_mode(false)
      , _override_liquid_id(true)
      , _override_height(true)
      , _opacity_mode(river_opacity)
      , _custom_opacity_factor(0.0337f)
      , _lock_pos(math::vector_3d(0.0f, 0.0f, 0.0f))
      , tile(0, 0)
    {
      auto layout (new QFormLayout (this));

      auto brush_group (new QGroupBox (this));
      auto brush_layout (new QFormLayout (brush_group));

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.f, 250.f);
      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _radius = f; }
              );
      _radius_spin->setValue(_radius);
      brush_layout->addRow ("Radius", _radius_spin);

      waterType = new QComboBox(this);

      for (DBCFile::Iterator i = gLiquidTypeDB.begin(); i != gLiquidTypeDB.end(); ++i)
      {
        int liquid_id = i->getInt(LiquidTypeDB::ID);

        std::stringstream ss;
        ss << liquid_id << "-" << LiquidTypeDB::getLiquidName(liquid_id);
        waterType->addItem (QString::fromUtf8(ss.str().c_str()), QVariant (liquid_id));

      }

      connect (waterType, qOverload<int> (&QComboBox::currentIndexChanged)
              , [&]
                {
                  changeWaterType(waterType->currentData().toInt());
                }
              );

      brush_layout->addRow (waterType);

      layout->addRow (brush_group);

      auto angle_group (new QGroupBox ("Angled mode", this));
      angle_group->setCheckable (true);
      angle_group->setChecked (_angled_mode.get());
      
      
      connect ( &_angled_mode, &bool_toggle_property::changed
              , angle_group, &QGroupBox::setChecked
              );
      connect ( angle_group, &QGroupBox::toggled
              , &_angled_mode, &bool_toggle_property::set
              );
      auto angle_layout (new QFormLayout (angle_group));

      _angle_spin = new QDoubleSpinBox (this);
      _angle_spin->setRange (0.00001f, 89.f);
      _angle_spin->setSingleStep (2.0f);
      connect ( _angle_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _angle = f; }
              );
      _angle_spin->setValue(_angle);
      angle_layout->addRow ("Angle", _angle_spin);

      _orientation_spin = new QDoubleSpinBox (this);
      _orientation_spin->setRange (0.f, 360.f);
      _orientation_spin->setWrapping (true);
      _orientation_spin->setValue(_orientation);
      _orientation_spin->setSingleStep (5.0f);
      connect ( _orientation_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _orientation = f; }
              );

      angle_layout->addRow ("Orienation", _orientation_spin);

      layout->addRow (angle_group);

      auto lock_group (new QGroupBox ("Lock", this));
      lock_group->setCheckable (true);
      lock_group->setChecked (_locked.get());
      auto lock_layout (new QFormLayout (lock_group));

      lock_layout->addRow("X:", _x_spin = new QDoubleSpinBox (this));
      lock_layout->addRow("Z:", _z_spin = new QDoubleSpinBox (this));
      lock_layout->addRow("H:", _h_spin = new QDoubleSpinBox (this));

      _x_spin->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _z_spin->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _h_spin->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _x_spin->setDecimals (2);
      _z_spin->setDecimals (2);
      _h_spin->setDecimals (2);

      connect ( _x_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _lock_pos.x = f; }
              );
      connect ( _z_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _lock_pos.z = f; }
              );
      connect ( _h_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _lock_pos.y = f; }
              );

      connect ( &_locked, &bool_toggle_property::changed
              , lock_group, &QGroupBox::setChecked
              );
      connect ( lock_group, &QGroupBox::toggled
              , &_locked, &bool_toggle_property::set
              );

      layout->addRow(lock_group);

      auto override_group (new QGroupBox ("Override", this));
      auto override_layout (new QFormLayout (override_group));

      override_layout->addWidget (new checkbox ("Liquid ID", &_override_liquid_id, this));
      override_layout->addWidget (new checkbox ("Height", &_override_height, this));

      layout->addRow(override_group);

      auto opacity_group (new QGroupBox ("Auto opacity", this));
      auto opacity_layout (new QFormLayout (opacity_group));

      auto river_button (new QRadioButton ("River", this));
      auto ocean_button (new QRadioButton ("Ocean", this));
      auto custom_button (new QRadioButton ("Custom factor:", this));

      QButtonGroup *transparency_toggle = new QButtonGroup (this);
      transparency_toggle->addButton (river_button, river_opacity);
      transparency_toggle->addButton (ocean_button, ocean_opacity);
      transparency_toggle->addButton (custom_button, custom_opacity);

      connect ( transparency_toggle, qOverload<int> (&QButtonGroup::buttonClicked)
              , [&] (int id) { _opacity_mode = id; }
              );

      opacity_layout->addRow (river_button);
      opacity_layout->addRow (ocean_button);
      opacity_layout->addRow (custom_button);

      transparency_toggle->button (river_opacity)->setChecked (true);

      QDoubleSpinBox *opacity_spin = new QDoubleSpinBox (this);
      opacity_spin->setRange (0.f, 1.f);
      opacity_spin->setDecimals (4);
      opacity_spin->setSingleStep (0.02f);
      opacity_spin->setValue(_custom_opacity_factor);
      connect ( opacity_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _custom_opacity_factor = f; }
              );
      opacity_layout->addRow (opacity_spin);

      layout->addRow (opacity_group);

      layout->addRow ( new pushbutton
                            ( "Regen ADT opacity"
                            , [this]
                              {
                                emit regenerate_water_opacity
                                  (get_opacity_factor());
                              }
                            )
                        );
      layout->addRow ( new pushbutton
                            ( "Crop water"
                            , [this]
                              {
                                emit crop_water();
                              }
                            )
                        );

      auto layer_group (new QGroupBox ("Layers", this));
      auto layer_layout (new QFormLayout (layer_group));

      layer_layout->addRow (new checkbox("Show all layers", display_all_layers));
      layer_layout->addRow (new QLabel("Current layer:", this));

      waterLayer = new QSpinBox (this);
      waterLayer->setValue (current_layer->get());
      waterLayer->setRange (0, 100);
      layer_layout->addRow (waterLayer);

      layout->addRow (layer_group);

      connect ( waterLayer, qOverload<int> (&QSpinBox::valueChanged)
              , current_layer, &unsigned_int_property::set
              );
      connect ( current_layer, &unsigned_int_property::changed
              , waterLayer, &QSpinBox::setValue
              );

      updateData();
      setMinimumWidth(sizeHint().width());
    }

    void water::updatePos(tile_index const& newTile)
    {
      if (newTile == tile) return;

      tile = newTile;

      updateData();
    }

    void water::updateData()
    {
      std::stringstream mt;
      mt << _liquid_id << " - " << LiquidTypeDB::getLiquidName(_liquid_id);
      waterType->setCurrentText (QString::fromStdString (mt.str()));
    }

    void water::changeWaterType(int waterint)
    {
      _liquid_id = waterint;
      updateData();
    }

    void water::changeRadius(float change)
    {
      _radius_spin->setValue(_radius + change);
    }

    void water::changeOrientation(float change)
    {
      _orientation += change;

      while (_orientation >= 360.0f)
      {
        _orientation -= 360.0f;
      }
      while (_orientation < 0.0f)
      {
        _orientation += 360.0f;
      }

      _orientation_spin->setValue(_orientation);
    }

    void water::changeAngle(float change)
    {
      _angle_spin->setValue(_angle + change);
    }

    void water::change_height(float change)
    { 
      _h_spin->setValue(_lock_pos.y + change);
    }

    void water::paintLiquid (World* world, math::vector_3d const& pos, bool add)
    {
      world->paintLiquid ( pos
                         , _radius
                         , _liquid_id
                         , add
                         , math::degrees (_angled_mode.get() ? _angle : 0.0f)
                         , math::degrees (_angled_mode.get() ? _orientation : 0.0f)
                         , _locked.get()
                         , _lock_pos
                         , _override_height.get()
                         , _override_liquid_id.get()
                         , get_opacity_factor()
                         );
    }

    void water::lockPos(math::vector_3d const& cursor_pos)
    {
      QSignalBlocker const blocker_x(_x_spin);
      QSignalBlocker const blocker_z(_z_spin);
      QSignalBlocker const blocker_h(_h_spin);
      _lock_pos = cursor_pos;

      _x_spin->setValue(_lock_pos.x);
      _z_spin->setValue(_lock_pos.z);
      _h_spin->setValue(_lock_pos.y);

      if (!_locked.get())
      {
        toggle_lock();
      }
    }

    void water::toggle_lock()
    {
      _locked.toggle();
    }

    void water::toggle_angled_mode()
    {
      _angled_mode.toggle();
    }

    float water::get_opacity_factor() const
    {
      switch (_opacity_mode)
      {
      default:          // values found by experimenting
      case river_opacity:  return 0.0337f;
      case ocean_opacity:  return 0.007f;
      case custom_opacity: return _custom_opacity_factor;
      }
    }

    QSize water::sizeHint() const
    {
      return QSize(215, height());
    }
  }
}
