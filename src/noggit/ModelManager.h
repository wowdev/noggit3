// ModelManager.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <string>
#include <map>

class Model;

class ModelManager
{
public:
  static void resetAnim();
  static void updateEmitters(float dt);

  static void report();

private:
  friend struct scoped_model_reference;
  static Model* add(std::string name);
  static void delbyname(std::string name);

  typedef std::map<std::string, Model*> mapType;
  static mapType items;
};

struct scoped_model_reference
{
  scoped_model_reference (std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _model (ModelManager::add (_filename))
  {}

  scoped_model_reference (scoped_model_reference const& other)
    : scoped_model_reference (other._filename)
  {}
  scoped_model_reference (scoped_model_reference&& other)
    : _valid (std::move (other._valid))
    , _filename (std::move (other._filename))
    , _model (std::move (other._model))
  {
    other._valid = false;
  }
  scoped_model_reference& operator= (scoped_model_reference const&) = delete;
  scoped_model_reference& operator= (scoped_model_reference&& other)
  {
    std::swap (_valid, other._valid);
    std::swap (_filename, other._filename);
    std::swap (_model, other._model);
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

#endif// MODELMANAGER_H
