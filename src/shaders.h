#ifndef __SHADERS_H
#define __SHADERS_H

#include <string>

#include "video.h"

#include "mpq.h"
#include "noggit.h"

class BLSShader
{
private:
	GLuint mShader;
	bool mOkay;
	int mProgramType;
	
public: 
	BLSShader( std::string pFilename );
	
	void EnableShader()
	{
		if( IsOkay() )
			glBindProgramARB( mProgramType, mShader );
	}
	
	bool IsOkay() { return mOkay; }
};


//Shaders WoWMapView 3.x

void initShaders();
void reloadShaders();

class Shader {
	GLenum target;
	GLuint id;
public:
	bool ok;

	Shader(GLenum target, const char *program, bool fromFile = false);
	~Shader();

	virtual void bind();
	virtual void unbind();
};

class ShaderPair {
	Shader *vertex;
	Shader *fragment;
public:

	ShaderPair():vertex(0),fragment(0) {}
	ShaderPair(Shader *vs, Shader *ps):vertex(vs), fragment(ps) {}
	ShaderPair(const char *vprog, const char *fprog, bool fromFile = false);

	void bind();
	void unbind();
};

extern ShaderPair *terrainShaders[4], *wmoShader, *waterShaders[1];

#endif
