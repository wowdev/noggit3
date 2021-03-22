// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

#include <noggit/scripting/script_object.hpp>

#include <math/vector_3d.hpp>

#include <boost/variant.hpp>

class World;

class ModelInstance;
class WMOInstance;

namespace noggit
{
  namespace scripting
  {
    class scripting_tool;
    class script_context;
    class model : public script_object
    {
    public:
      model(script_context * ctx, ModelInstance* model);
      model(script_context * ctx, WMOInstance* model);

      math::vector_3d get_pos();
      void set_pos(math::vector_3d& pos);

      math::vector_3d get_rot();
      void set_rot(math::vector_3d& rot);

      float get_scale();
      void set_scale(float scale);

      unsigned get_uid();

      void remove();

      std::string get_filename();
      void replace(std::string const& filename);

    private:
      boost::variant<ModelInstance*, WMOInstance*> _impl;
    };

    class model_iterator: public script_object
    {
    public:
      model_iterator(script_context * ctx
                    , math::vector_3d min
                    , math::vector_3d max
                    );

      bool next();
      void reset();
      void query();
      model get();

    private:
      math::vector_3d _min;
      math::vector_3d _max;
      std::vector<model> _models;

      bool _initialized = false;
      int _model_index = -1;
    };

    void register_model(script_context * state);
  } // namespace scripting
} // namespace noggit
