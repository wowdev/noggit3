// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ModelManager.h>

#include <noggit/application.h>

namespace noggit
{
  model_manager::model_manager()
    : multimap_with_normalized_key<Model>
        ( [] (std::string filename)
          {
            filename = mpq::normalized_filename (filename);

            std::size_t found;
            if ( (found = filename.rfind (".mdx")) != std::string::npos
              || (found = filename.rfind (".mdl")) != std::string::npos
               )
            {
              filename.replace( found, 4, ".m2" );
            }

            return filename;
          }
        )
  {}

  void model_manager::resetAnim()
  {
    apply ( [&] (std::string const&, Model& model)
            {
              model.animcalc = false;
            }
          );
  }

  void model_manager::updateEmitters( float dt )
  {
    apply ( [&] (std::string const&, Model& model)
            {
              model.updateEmitters (dt);
            }
          );
  }

  scoped_model_reference::scoped_model_reference (std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _model (noggit::app().model_manager().emplace (_filename))
  {}

  scoped_model_reference::scoped_model_reference (scoped_model_reference const& other)
    : scoped_model_reference (other._filename)
  {}
  scoped_model_reference::scoped_model_reference (scoped_model_reference&& other)
    : _valid (std::move (other._valid))
    , _filename (std::move (other._filename))
    , _model (std::move (other._model))
  {
    other._valid = false;
  }
  scoped_model_reference& scoped_model_reference::operator= (scoped_model_reference&& other)
  {
    std::swap (_valid, other._valid);
    std::swap (_filename, other._filename);
    std::swap (_model, other._model);
    return *this;
  }

  scoped_model_reference::~scoped_model_reference()
  {
    if (_valid)
    {
      noggit::app().model_manager().erase (_filename);
    }
  }
}
