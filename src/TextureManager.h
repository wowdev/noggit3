#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

class Texture;
class TextureManager;

#include "manager.h"
#include <GL/glew.h>

class Texture : public ManagedItem 
{
	//! \todo  Private members?
public:
	int w,h;
	GLuint id;

	Texture(const std::string& pname): ManagedItem(pname), w(0), h(0) {}
};

class TextureManager : public Manager<GLuint> 
{
	bool LoadBLP(GLuint id, Texture *tex);

public:
	void reload();
	virtual GLuint add(const std::string& name);
	void doDelete(GLuint id);
	GLuint get(const std::string& name);
};

#endif
