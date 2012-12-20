#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <string>
#include <map>

class Model;

class ModelManager
{
public:
  static void delbyname( std::string name );
  static Model* add(std::string name);

  static void resetAnim();
  static void updateEmitters(float dt);

  static void report();

private:
  typedef std::map<std::string, Model*> mapType;
  static mapType items;
};

#endif// MODELMANAGER_H
