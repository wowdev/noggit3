#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

class TextureManager;

#include <GL/glew.h>
#include <map>
#include <string>

#include "Manager.h" // ManagedItem
#include "Video.h"

class TextureManager : public Manager<GLuint,OpenGL::Texture>
{
private:
  static bool LoadBLP(GLuint id, OpenGL::Texture *tex);
public:
  static void reload();
  static GLuint add(std::string name);
  static void doDelete(GLuint id);
  static GLuint get(const std::string& name);
  
  static OpenGL::Texture* newTexture(const std::string& name);
};

#endif
