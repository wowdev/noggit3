// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/ui/Water.h>
#include <noggit/ui/pushbutton.hpp>
#include <util/qt/overload.hpp>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QRadioButton>

namespace noggit
{
  namespace ui
  {
    water::water ( unsigned_int_property* current_layer
                 , bool_toggle_property* display_all_layers
                 )
      : QWidget (nullptr)
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
      auto layout (new QVBoxLayout (this));

      auto spinners_layout (new QFormLayout);

      _radius_spin = new QDoubleSpinBox (this);
      _radius_spin->setRange (0.f, 250.f);
      connect ( _radius_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _radius = f; }
              );
      _radius_spin->setValue(_radius);
      spinners_layout->addRow ("Radius", _radius_spin);

      _angle_spin = new QDoubleSpinBox (this);
      _angle_spin->setRange (0.00001f, 89.f);
      connect ( _angle_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _angle = f; }
              );
      _angle_spin->setValue(_angle);
      spinners_layout->addRow ("Angle", _angle_spin);

      _orientation_spin = new QDoubleSpinBox (this);
      _orientation_spin->setRange (0.f, 359.99999f);
      connect ( _orientation_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _orientation = f; }
              );
      _orientation_spin->setValue(_orientation);
      spinners_layout->addRow ("Orienation", _orientation_spin);

      layout->addLayout (spinners_layout);

      layout->addWidget (_angle_checkbox = new checkbox ("Angled water", &_angled_mode, this));
      layout->addWidget (_lock_checkbox = new checkbox ("Lock position", &_locked, this));

      waterType = new pushbutton
        ( "Type: none"
        , [this]
          {
            QListWidget* water_type_browser (new QListWidget (nullptr));
            connect (this, &QObject::destroyed, water_type_browser, &QObject::deleteLater);

            water_type_browser->setWindowTitle("Water type selector");
            water_type_browser->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

            for (DBCFile::Iterator i = gLiquidTypeDB.begin(); i != gLiquidTypeDB.end(); ++i)
            {
              int liquid_id = i->getInt(LiquidTypeDB::ID);

              std::stringstream ss;
              ss << liquid_id << "-" << LiquidTypeDB::getLiquidName(liquid_id);

              auto item (new QListWidgetItem (QString::fromUtf8 (ss.str().c_str()), water_type_browser));
              item->setData (Qt::UserRole, QVariant (liquid_id));

              water_type_browser->addItem(item);
            }

            connect ( water_type_browser, &QListWidget::itemClicked
                    , [&] (QListWidgetItem* item)
                      {
                        changeWaterType(item->data(Qt::UserRole).toInt());
                      }
                    );

            water_type_browser->show();
          }
        , this
        );

      layout->addWidget (waterType);

      layout->addWidget (new QLabel ("Override :"));

      layout->addWidget (new checkbox ("Liquid ID", &_override_liquid_id, this));
      layout->addWidget (new checkbox ("Height", &_override_height, this));

      layout->addWidget (new QLabel ("Auto opacity:"));

      auto river_button (new QRadioButton ("River", this));
      auto ocean_button (new QRadioButton ("Ocean", this));
      auto custom_button (new QRadioButton ("Custom", this));

      QButtonGroup *transparency_toggle = new QButtonGroup (this);
      transparency_toggle->addButton (river_button, river_opacity);
      transparency_toggle->addButton (ocean_button, ocean_opacity);
      transparency_toggle->addButton (custom_button, custom_opacity);

      connect ( transparency_toggle, qOverload<int> (&QButtonGroup::buttonClicked)
              , [&] (int id) { _opacity_mode = id; }
              );

      layout->addWidget (river_button);
      layout->addWidget (ocean_button);
      layout->addWidget (custom_button);

      transparency_toggle->button (river_opacity)->setChecked (true);

      layout->addWidget (new QLabel ("custom factor", this));

      QDoubleSpinBox *opacity_spin = new QDoubleSpinBox (this);
      opacity_spin->setRange (0.f, 1.f);
      opacity_spin->setDecimals (4);
      opacity_spin->setValue(_custom_opacity_factor);
      connect ( opacity_spin, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (float f) { _custom_opacity_factor = f; }
              );
      layout->addWidget (opacity_spin);

      layout->addWidget ( new pushbutton
                            ( "Regen ADT opacity"
                            , [this]
                              {
                                emit regenerate_water_opacity
                                  (get_opacity_factor());
                              }
                            )
                        );
      layout->addWidget ( new pushbutton
                            ( "Crop water"
                            , [this]
                              {
                                emit crop_water();
                              }
                            )
                        );

      layout->addWidget (new checkbox("Show all layers", display_all_layers));

      layout->addWidget (new QLabel("Current layer:", this));

      waterLayer = new QSpinBox (this);
      waterLayer->setValue (current_layer->get());
      waterLayer->setRange (0, 100);
      layout->addWidget (waterLayer);
      connect ( waterLayer, qOverload<int> (&QSpinBox::valueChanged)
              , current_layer, &unsigned_int_property::set
              );
      connect ( current_layer, &unsigned_int_property::changed
              , waterLayer, &QSpinBox::setValue
              );

      updateData();
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
      waterType->setText (QString::fromStdString (mt.str()));
    }

    void water::changeWaterType(int waterint)
    {
      _liquid_id = waterint;
      updateData();
    }

    void water::changeRadius(float change)
    {
      _angle = std::max(0.0f, std::min(250.0f, _radius + change));
      _radius_spin->setValue(_radius / 250.0f);
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

      _orientation_spin->setValue(_orientation / 360.0f);
    }

    void water::changeAngle(float change)
    {
      _angle = std::max(0.0f, std::min(89.0f, _angle + change));
      _angle_spin->setValue(_angle / 90.0f);
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
      _lock_pos = cursor_pos;

      if (!_locked.get())
      {
        toggle_lock();
      }
    }

    void water::toggle_lock()
    {
      _locked.set (!_locked.get());
    }

    void water::toggle_angled_mode()
    {
      _angled_mode.set (!_angled_mode.get());
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
  }
}
