// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/RotationEditor.h>

#include <noggit/Misc.h>
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>
#include <util/qt/overload.hpp>
#include <noggit/ui/ObjectEditor.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>

namespace noggit
{
  namespace ui
  {
    rotation_editor::rotation_editor(QWidget* parent, World* world)
      : QWidget (parent)
    {
      setWindowTitle("Pos/Rotation Editor");
      setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

      auto layout (new QFormLayout (this));

      layout->addRow (new QLabel ("Tilt", this));
      layout->addRow ("X", _rotation_x = new QDoubleSpinBox (this));
      layout->addRow ("Z", _rotation_z = new QDoubleSpinBox (this));

      layout->addRow (new QLabel ("Rotation", this));
      layout->addRow ("", _rotation_y = new QDoubleSpinBox (this));

      layout->addRow (new QLabel ("Position", this));
      layout->addRow ("X", _position_x = new QDoubleSpinBox (this));
      layout->addRow ("Z", _position_z = new QDoubleSpinBox (this));
      layout->addRow ("H", _position_y = new QDoubleSpinBox (this));

      layout->addRow (new QLabel ("Scale", this));
      layout->addRow ("", _scale = new QDoubleSpinBox (this));

      layout->addRow(new QLabel("Multi selection warning:", this));
      layout->addRow(new QLabel("- rotation and scale only\n  change when pressing enter", this));
      layout->addRow(new QLabel("- scaling is multiplicative", this));

      _rotation_x->setRange (-180.f, 180.f);
      _rotation_x->setDecimals (3);
      _rotation_x->setWrapping(true);
      _rotation_x->setSingleStep(5.0f);
      _rotation_z->setRange (-180.f, 180.f);
      _rotation_z->setDecimals (3);
      _rotation_z->setWrapping(true);
      _rotation_z->setSingleStep(5.0f);
      _rotation_y->setRange (-180.f, 180.f);
      _rotation_y->setDecimals (3);
      _rotation_y->setWrapping(true);
      _rotation_y->setSingleStep(5.0f);

      _position_x->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_x->setDecimals (5);
      _position_z->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_z->setDecimals (5);
      _position_y->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_y->setDecimals (5);

      _scale->setRange (ModelInstance::min_scale(), ModelInstance::max_scale());
      _scale->setDecimals (2);
      _scale->setSingleStep(0.1f);


      connect ( _rotation_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world] { set_model_rotation(world); }
              );
      connect ( _rotation_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world] { set_model_rotation(world); }
              );
      connect ( _rotation_y, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world] { set_model_rotation(world); }
              );

      connect ( _rotation_x, &QDoubleSpinBox::editingFinished
              , [&, world]
                {
                  if (world->has_multiple_model_selected())
                  {
                    // avoid rotation changes when losing focus
                    if (_rotation_x->hasFocus())
                    {
                      change_models_rotation(world);
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_rotation_x);
                      _rotation_x->setValue(0.f);
                    }
                  }
                }
              );
      connect ( _rotation_z, &QDoubleSpinBox::editingFinished
              , [&, world]
                {
                  if (world->has_multiple_model_selected())
                  {
                    // avoid rotation changes when losing focus
                    if (_rotation_z->hasFocus())
                    {
                      change_models_rotation(world);
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_rotation_z);
                      _rotation_z->setValue(0.f);
                    }
                  }
                }
              );
      connect ( _rotation_y, &QDoubleSpinBox::editingFinished
              , [&, world]
                {
                  if (world->has_multiple_model_selected())
                  {
                    // avoid rotation changes when losing focus
                    if (_rotation_y->hasFocus())
                    {
                      change_models_rotation(world);
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_rotation_y);
                      _rotation_y->setValue(0.f);
                    }
                  }
                }
              );

      connect ( _position_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world] (double v)
                {
                  world->set_selected_models_pos(v, _position_y->value(), _position_z->value());
                }
              );
      connect ( _position_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world] (double v)
                {
                  world->set_selected_models_pos(_position_x->value(), _position_y->value(), v);
                }
              );
      connect ( _position_y, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&, world] (double v)
                {
                  world->set_selected_models_pos(_position_x->value(), v, _position_z->value());
                }
              );

      connect ( _scale, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [world] (double v)
                {
                  if (!world->has_multiple_model_selected())
                  {
                    world->scale_selected_models(v, World::m2_scaling_type::set);
                  }
                }
              );
      connect (_scale, &QDoubleSpinBox::editingFinished
              , [&, world]
                {
                  if(world->has_multiple_model_selected())
                  {
                    // avoid scale changes when losing focus
                    if (_scale->hasFocus())
                    {
                      world->scale_selected_models(_scale->value(), World::m2_scaling_type::mult);
                    }
                    else // reset value
                    {
                      QSignalBlocker const _(_scale);
                      _scale->setValue(1.f);
                    }
                  }
                }
              );
    }

    void rotation_editor::updateValues(World* world)
    {
      QSignalBlocker const block_rotation_x(_rotation_x);
      QSignalBlocker const block_rotation_y(_rotation_y);
      QSignalBlocker const block_rotation_z(_rotation_z);
      QSignalBlocker const block_position_x(_position_x);
      QSignalBlocker const block_position_y(_position_y);
      QSignalBlocker const block_position_z(_position_z);
      QSignalBlocker const block_scale(_scale);

      if (world->has_multiple_model_selected())
      {
        math::vector_3d const& p = world->multi_select_pivot().get();

        _position_x->setValue(p.x);
        _position_y->setValue(p.y);
        _position_z->setValue(p.z);

        _position_x->setEnabled(true);
        _position_y->setEnabled(true);
        _position_z->setEnabled(true);
        // default value for rotation and scaling, affect the models only when pressing enter
        _rotation_x->setValue(0.f);
        _rotation_y->setValue(0.f);
        _rotation_z->setValue(0.f);
        _rotation_x->setEnabled(true);
        _rotation_y->setEnabled(true);
        _rotation_z->setEnabled(true);
        _scale->setValue(1.f);
        _scale->setEnabled(true);
      }
      else
      {
        auto entry = world->get_last_selected_model();

        if (entry)
        {
          selection_type selection = entry.get();

          if (selection.which() == eEntry_Model)
          {
            auto model = boost::get<selected_model_type>(selection);
            _position_x->setValue(model->pos.x);
            _position_y->setValue(model->pos.y);
            _position_z->setValue(model->pos.z);
            _rotation_x->setValue(model->dir.x._);
            _rotation_y->setValue(model->dir.y._);
            _rotation_z->setValue(model->dir.z._);
            _scale->setValue(model->scale);

            _scale->setEnabled(true);
          }
          else // we know it's a wmo
          {
            auto wmo = boost::get<selected_wmo_type>(selection);
            _position_x->setValue(wmo->pos.x);
            _position_y->setValue(wmo->pos.y);
            _position_z->setValue(wmo->pos.z);
            _rotation_x->setValue(wmo->dir.x._);
            _rotation_y->setValue(wmo->dir.y._);
            _rotation_z->setValue(wmo->dir.z._);

            _scale->setValue(1.f);
            _scale->setEnabled(false);
          }

          _rotation_x->setEnabled(true);
          _rotation_y->setEnabled(true);
          _rotation_z->setEnabled(true);
          _position_x->setEnabled(true);
          _position_y->setEnabled(true);
          _position_z->setEnabled(true);
        }
        else
        {
          _rotation_x->setEnabled(false);
          _rotation_y->setEnabled(false);
          _rotation_z->setEnabled(false);
          _position_x->setEnabled(false);
          _position_y->setEnabled(false);
          _position_z->setEnabled(false);
          _scale->setEnabled(false);

          _rotation_x->setValue(0.f);
          _rotation_y->setValue(0.f);
          _rotation_z->setValue(0.f);
          _position_x->setValue(0.f);
          _position_y->setValue(0.f);
          _position_z->setValue(0.f);
          _scale->setValue(1.f);
        }
      }
    }

    void rotation_editor::set_model_rotation(World* world)
    {
      // only for single model rotation
      if (!world->has_multiple_model_selected())
      {
        world->set_selected_models_rotation
          ( math::degrees(_rotation_x->value())
          , math::degrees(_rotation_y->value())
          , math::degrees(_rotation_z->value())
          );
      }
    }

    void rotation_editor::change_models_rotation(World* world)
    {
      // only for multi models rotation
      if (world->has_multiple_model_selected())
      {
        world->rotate_selected_models
          ( math::degrees(_rotation_x->value())
          , math::degrees(_rotation_y->value())
          , math::degrees(_rotation_z->value())
          , *use_median_pivot_point
          );
      }
    }
  }
}
