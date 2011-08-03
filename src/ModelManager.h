#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <string>

#include "Manager.h" // Manager
#include "Model.h" // Model

typedef unsigned int MODELIDTYPE;

class ModelManager: public Manager<MODELIDTYPE,Model>
{
private:
  static int baseid;
public:
  static MODELIDTYPE add(std::string name);
  
  static void resetAnim();
  static void updateEmitters(float dt);
  static int nextID()
  {
    return baseid++;
  }
};

#endif// MODELMANAGER_H