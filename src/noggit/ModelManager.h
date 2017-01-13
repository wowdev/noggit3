// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>
#include <map>
#include <vector>

class Model;

class ModelManager
{
public:
	static void resetAnim();
	static void updateEmitters(float dt);

	static void report();
  static void toggleModelVisibility(Model* model);
  static void clearHiddenModelList();

private:
  friend struct scoped_model_reference;
  static Model* add(std::string name);
  static void delbyname(std::string name);

	typedef std::map<std::string, Model*> mapType;
	static mapType items;
  typedef std::vector<Model*> vectorType;
  static vectorType hiddenItems;
};

struct scoped_model_reference
{
  scoped_model_reference (std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _model (ModelManager::add (_filename))
  {}

  scoped_model_reference (scoped_model_reference const& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _model (ModelManager::add (_filename))
  {}
  scoped_model_reference& operator= (scoped_model_reference const& other)
  {
    _valid = other._valid;
    _filename = other._filename;
    _model = ModelManager::add (_filename);
    return *this;
  }

  scoped_model_reference (scoped_model_reference&& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _model (other._model)
  {
    other._valid = false;
  }
  scoped_model_reference& operator= (scoped_model_reference&& other)
  {
    std::swap (_valid, other._valid);
    std::swap (_filename, other._filename);
    std::swap (_model, other._model);
    other._valid = false;
    return *this;
  }

  ~scoped_model_reference()
  {
    if (_valid)
    {
      ModelManager::delbyname (_filename);
    }
  }

  Model* operator->() const
  {
    return _model;
  }

private:
  bool _valid;
  std::string _filename;
  Model* _model;
};
