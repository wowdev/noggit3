#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

class TextureManager;

#include <map>
#include <string>
#include <GL/glew.h>

#include "manager.h" // ManagedItem
#include "video.h"

class TextureManager : public Manager<GLuint,OpenGL::Texture>
{
private:
	static bool LoadBLP(GLuint id, OpenGL::Texture *tex);
public:
	static void reload();
	static GLuint add(const std::string& name);
	static void doDelete(GLuint id);
	static GLuint get(const std::string& name);
	
	static OpenGL::Texture* newTexture(const std::string& name)
	{
		return (OpenGL::Texture*)TextureManager::items[TextureManager::add( name )];
	}
};

#endif
