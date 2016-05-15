// This file is part of Noggit3, licensed via GNU General Public License (version 3).

#pragma once

#include <noggit/Model.h>
#include <noggit/multimap_with_normalized_key.hpp>

class Model;

namespace noggit
{
  struct model_manager : private multimap_with_normalized_key<Model>
  {
    model_manager();

    void resetAnim();
    void updateEmitters (float dt);

    friend struct scoped_model_reference;
  };

  struct scoped_model_reference
  {
    scoped_model_reference (std::string const& filename);

    scoped_model_reference (scoped_model_reference const& other);
    scoped_model_reference (scoped_model_reference&& other);
    scoped_model_reference& operator= (scoped_model_reference const&) = delete;
    scoped_model_reference& operator= (scoped_model_reference&& other);

    ~scoped_model_reference();

    Model* operator->() const
    {
      return _model;
    }

  private:
    bool _valid;
    std::string _filename;
    Model* _model;
  };
}
