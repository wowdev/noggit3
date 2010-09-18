#ifndef VIDEO_H
#define VIDEO_H

class Video;

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <SDL/SDL.h>

#include "TextureManager.h"

void SaveGLSettings();
void LoadGLSettings();

class Video
{
	int status;
	int	flags;

	SDL_Surface *primary;

public:
	bool fullscreen;

	bool init(int xres, int yres, bool fullscreen);

	void close();

	void flip();
	void clearScreen();
	void set3D();
	void set3D_select();
	void set2D();
	void setTileMode();
	void resize(int w, int h);

	TextureManager textures;
    
	int xres, yres;
	int origX, origY;
	float ratio;

	/// is * supported:
	bool mSupportShaders;
	bool mSupportCompression;
};

extern Video video;

//GLuint loadTGA(const char *filename, bool mipmaps);
bool isExtensionSupported(const char *search);
void CheckForGLError( const std::string& pLocation );

#endif
