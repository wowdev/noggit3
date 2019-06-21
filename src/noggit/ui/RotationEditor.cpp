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


      _rotation_x->setRange (-180.f, 180.f);
      _rotation_x->setDecimals (3);
      _rotation_x->setWrapping(true);
      _rotation_x->setSingleStep(5.0f);
      _rotation_z->setRange (-180.f, 180.f);
      _rotation_z->setDecimals (3);
      _rotation_z->setWrapping(true);
      _rotation_z->setSingleStep(5.0f);
      _rotation_y->setRange (0.f, 360.f);
      _rotation_y->setDecimals (3);
      _rotation_y->setWrapping(true);
      _rotation_y->setSingleStep(5.0f);

      _position_x->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_x->setDecimals (5);
      _position_z->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_z->setDecimals (5);
      _position_y->setRange (std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
      _position_y->setDecimals (5);

      _scale->setRange (ModelInstance::min_scale, ModelInstance::max_scale);
      _scale->setDecimals (2);
      _scale->setSingleStep(0.1f);


      connect ( _rotation_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  auto lastEntry = _entries.back();
                  double difference = v;
                  if (lastEntry.which() == eEntry_Model)
                  {
                    difference = v - boost::get<selected_model_type>(lastEntry)->dir.x;
                  }
                  else if (lastEntry.which() == eEntry_WMO)
                  {
                    difference = v - boost::get<selected_wmo_type>(lastEntry)->dir.x;
                  }

                  if (_entries.size() > 1 && *use_median_pivot_point)
                  {
                    auto rotationPivotPoint = getMedianPivotPoint(_entries);

                    for (auto& selection : _entries)
                    {
                      math::vector_3d* position = nullptr;
                      math::vector_3d* rotation = nullptr;

                      if (selection.which() == eEntry_Model)
                      {
                        position = &boost::get<selected_model_type>(selection)->pos;
                        rotation = &boost::get<selected_model_type>(selection)->dir;
                      }
                      else if (selection.which() == eEntry_WMO)
                      {
                        position = &boost::get<selected_wmo_type>(selection)->pos;
                        rotation = &boost::get<selected_wmo_type>(selection)->dir;
                      }

                      auto oldPos = *position;

                      rotateByXAxis(selection, rotationPivotPoint, difference * math::constants::pi / 180);
                      rotation->x -= calculateRotationXAngle(*position, oldPos, rotationPivotPoint);

                      if (rotation->x > 180.0f)
                        rotation->x -= 360.0f;
                      else if (rotation->x < -180.0f)
                        rotation->x += 360.0f;

                      update_model(selection);
                    }
                  }
                  else
                  {
                    for (auto& selection : _entries)
                    {
                      if (selection.which() == eEntry_Model)
                      {
                        auto model = boost::get<selected_model_type>(selection);
                        model->dir.x = v;
                      }
                      else if (selection.which() == eEntry_WMO)
                      {
                        auto wmo = boost::get<selected_wmo_type>(selection);
                        wmo->dir.x = v;
                      }
                      update_model(selection);
                    }
                  }
                }
              );
      connect ( _rotation_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  auto lastEntry = _entries.back();
                  double difference = v;
                  if (lastEntry.which() == eEntry_Model)
                  {
                    difference = v - boost::get<selected_model_type>(lastEntry)->dir.z;
                  }
                  else if (lastEntry.which() == eEntry_WMO)
                  {
                    difference = v - boost::get<selected_wmo_type>(lastEntry)->dir.z;
                  }

                  if (_entries.size() > 1 && *use_median_pivot_point)
                  {
                    auto rotationPivotPoint = getMedianPivotPoint(_entries);

                    for (auto& selection : _entries)
                    {
                      math::vector_3d* position = nullptr;
                      math::vector_3d* rotation = nullptr;

                      if (selection.which() == eEntry_Model)
                      {
                        position = &boost::get<selected_model_type>(selection)->pos;
                        rotation = &boost::get<selected_model_type>(selection)->dir;
                      }
                      else if (selection.which() == eEntry_WMO)
                      {
                        position = &boost::get<selected_wmo_type>(selection)->pos;
                        rotation = &boost::get<selected_wmo_type>(selection)->dir;
                      }

                      auto oldPos = *position;

                      rotateByZAxis(selection, rotationPivotPoint, difference * math::constants::pi / 180);
                      rotation->z -= calculateRotationZAngle(*position, oldPos, rotationPivotPoint);

                      if (rotation->z > 180)
                        rotation->z -= 360.0f;
                      else if (rotation->z < -180.0f)
                        rotation->z += 360.0f;

                      update_model(selection);
                    }
                  }
                  else
                  {
                    for (auto& selection : _entries)
                    {
                      if (selection.which() == eEntry_Model)
                      {
                        auto model = boost::get<selected_model_type>(selection);
                        model->dir.z = v;
                      }
                      else if (selection.which() == eEntry_WMO)
                      {
                        auto wmo = boost::get<selected_wmo_type>(selection);
                        wmo->dir.z = v;
                      }
                      update_model(selection);
                    }
                  }
                }
              );
      connect ( _rotation_y, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  auto lastEntry = _entries.back();
                  double difference = v;
                  if (lastEntry.which() == eEntry_Model)
                  {
                    difference = v - boost::get<selected_model_type>(lastEntry)->dir.y;
                  }
                  else if (lastEntry.which() == eEntry_WMO)
                  {
                    difference = v - boost::get<selected_wmo_type>(lastEntry)->dir.y;
                  }

                  if (_entries.size() > 1 && *use_median_pivot_point)
                  {
                    auto rotationPivotPoint = getMedianPivotPoint(_entries);

                    for (auto& selection : _entries)
                    {
                      math::vector_3d* position = nullptr;
                      math::vector_3d* rotation = nullptr;

                      if (selection.which() == eEntry_Model)
                      {
                        position = &boost::get<selected_model_type>(selection)->pos;
                        rotation = &boost::get<selected_model_type>(selection)->dir;
                      }
                      else if (selection.which() == eEntry_WMO)
                      {
                        position = &boost::get<selected_wmo_type>(selection)->pos;
                        rotation = &boost::get<selected_wmo_type>(selection)->dir;
                      }

                      auto oldPos = *position;

                      rotateByYAxis(selection, rotationPivotPoint, difference * math::constants::pi / 180);
                      rotation->y += calculateRotationYAngle(*position, oldPos, rotationPivotPoint);

                      if (rotation->y > 360.0f)
                        rotation->y -= 360.0f;
                      else if (rotation->y < 0.0f)
                        rotation->y += 360.0f;

                      update_model(selection);
                    }
                  }
                  else
                  {
                    for (auto& selection : _entries)
                    {
                      if (selection.which() == eEntry_Model)
                      {
                        auto model = boost::get<selected_model_type>(selection);
                        model->dir.y = v;
                      }
                      else if (selection.which() == eEntry_WMO)
                      {
                        auto wmo = boost::get<selected_wmo_type>(selection);
                        wmo->dir.y = v;
                      }
                      update_model(selection);
                    }
                  }
                }
              );

      connect ( _position_x, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  for (auto& selection : _entries)
                  {
                    if (selection.which() == eEntry_Model)
                    {
                      boost::get<selected_model_type>(selection)->pos.x = v;
                    }
                    else if (selection.which() == eEntry_WMO)
                    {
                      boost::get<selected_wmo_type>(selection)->pos.x = v;
                    }
                    update_model(selection);
                  }
                }
              );
      connect ( _position_z, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  for (auto& selection : _entries)
                  {
                    if (selection.which() == eEntry_Model)
                    {
                      boost::get<selected_model_type>(selection)->pos.z = v;
                    }
                    else if (selection.which() == eEntry_WMO)
                    {
                      boost::get<selected_wmo_type>(selection)->pos.z = v;
                    }
                    update_model(selection);
                  }
                }
              );
      connect ( _position_y, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  for (auto& selection : _entries)
                  {
                    if (selection.which() == eEntry_Model)
                    {
                      boost::get<selected_model_type>(selection)->pos.y = v;
                    }
                    else if (selection.which() == eEntry_WMO)
                    {
                      boost::get<selected_wmo_type>(selection)->pos.y = v;
                    }
                    update_model(selection);
                  }
                }
              );

      connect ( _scale, qOverload<double> (&QDoubleSpinBox::valueChanged)
              , [&] (double v)
                {
                  world->scale_selected_models(v, World::m2_scaling_type::set);
                }
              );
    }

    void rotation_editor::select(std::vector<selection_type> entries)
    {
      _entries.clear();

      if (entries.size() > 0)
      {
        _entries.insert(_entries.end(), entries.begin(), entries.end());
      }

      updateValues();
    }

    void rotation_editor::updateValues()
    {
      if (_entries.size() > 0)
      {
        QSignalBlocker const block_rotation_x (_rotation_x);
        QSignalBlocker const block_rotation_y (_rotation_y);
        QSignalBlocker const block_rotation_z (_rotation_z);
        QSignalBlocker const block_position_x (_position_x);
        QSignalBlocker const block_position_y (_position_y);
        QSignalBlocker const block_position_z (_position_z);
        QSignalBlocker const block_scale (_scale);

        auto lastEntry = _entries.back();

        if (lastEntry.which() == eEntry_Model)
        {
          auto model = boost::get<selected_model_type>(lastEntry);
          _rotation_x->setValue(model->dir.x);
          _rotation_y->setValue(model->dir.y);
          _rotation_z->setValue(model->dir.z);
          _position_x->setValue(model->pos.x);
          _position_y->setValue(model->pos.y);
          _position_z->setValue(model->pos.z);
          _scale->setValue(model->scale);

          _rotation_x->setEnabled (true);
          _rotation_y->setEnabled (true);
          _rotation_z->setEnabled (true);
          _position_x->setEnabled (true);
          _position_y->setEnabled (true);
          _position_z->setEnabled (true);
          _scale->setEnabled (true);
        }
        else if (lastEntry.which() == eEntry_WMO)
        {
          auto wmo = boost::get<selected_wmo_type>(lastEntry);
          _rotation_x->setValue(wmo->dir.x);
          _rotation_y->setValue(wmo->dir.y);
          _rotation_z->setValue(wmo->dir.z);
          _position_x->setValue(wmo->pos.x);
          _position_y->setValue(wmo->pos.y);
          _position_z->setValue(wmo->pos.z);

          _rotation_x->setEnabled(true);
          _rotation_y->setEnabled(true);
          _rotation_z->setEnabled(true);
          _position_x->setEnabled(true);
          _position_y->setEnabled(true);
          _position_z->setEnabled(true);
        }
      }
      else
      {
        _rotation_x->setEnabled (false);
        _rotation_y->setEnabled (false);
        _rotation_z->setEnabled (false);
        _position_x->setEnabled (false);
        _position_y->setEnabled (false);
        _position_z->setEnabled (false);
        _scale->setEnabled (false);
      }
    }

    void rotation_editor::update_model(selection_type entry)
    {
      if (entry.which() == eEntry_Model)
      {
        boost::get<selected_model_type>(entry)->recalcExtents();
      }
      else if (entry.which() == eEntry_WMO)
        {
        boost::get<selected_wmo_type>(entry)->recalcExtents();
        }
      }

    }
  }
