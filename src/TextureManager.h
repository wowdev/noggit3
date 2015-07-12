#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

#include <map>
#include <string>
#include <vector>

namespace OpenGL
{
	class Texture;
}

class TextureManager
{
public:
	static void delbyname(std::string name);
	static OpenGL::Texture* newTexture(std::string name);

	static void report();

	//! \todo This should not be there.
	//! \note This is only for getting all cached textures in UITexturingGUI.
	static std::vector<OpenGL::Texture*> getAllTexturesMatching(bool(*function)(const std::string& name));

private:
	typedef std::map<std::string, OpenGL::Texture*> mapType;
	static mapType items;
};

#endif
