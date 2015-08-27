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
    : _filename (filename)
    , _model (ModelManager::add (_filename))
  {}
  ~scoped_model_reference()
  {
    ModelManager::delbyname (_filename);
  }

  Model* operator->() const
  {
    return _model;
  }

private:
  std::string _filename;
  Model* _model;
};

#endif// MODELMANAGER_H
