#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

class Texture;
class TextureManager;

#include "manager.h"
#include <GL/glew.h>

class Texture : public ManagedItem 
{
	/// TODO: Private members?
public:
	int w,h;
	GLuint id;
	std::string originalName;

	Texture(std::string pname):ManagedItem(pname), w(0), h(0), originalName(pname) {}
};

class TextureManager : public Manager<GLuint> 
{
	bool LoadBLP(GLuint id, Texture *tex);

public:
	void reload();
	virtual GLuint add(std::string name);
	void doDelete(GLuint id);
	GLuint get(std::string name);
};

#endif