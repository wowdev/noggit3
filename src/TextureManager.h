#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

class Texture;
class TextureManager;

#include <map>
#include <string>
#include <GL/glew.h>

#include "manager.h" // ManagedItem

class Texture : public ManagedItem
{
private:
	int w,h;
	GLuint id;
  
public:
	Texture(const std::string& pname): ManagedItem(pname), w(0), h(0) {}
  const GLuint getId() const
  {
    return id;
  }
  void render() const
  {
    glBindTexture( GL_TEXTURE_2D, id );
  }
  
  static void enableTexture()
  {
    glEnable( GL_TEXTURE_2D );
  }
  static void disableTexture()
  {
    glDisable( GL_TEXTURE_2D );
  }
  static void setActiveTexture( size_t num = 0 )
  {
    glActiveTexture( GL_TEXTURE0 + num );
  }
  
  friend class TextureManager;
};

class TextureManager : public Manager<GLuint>
{
private:
	static bool LoadBLP(GLuint id, Texture *tex);
public:
	static void reload();
	static GLuint add(const std::string& name);
	static void doDelete(GLuint id);
	static GLuint get(const std::string& name);
  
  static Texture* newTexture(const std::string& name)
  {
    return (Texture*)TextureManager::items[TextureManager::add( name )];
  }
};

#endif
