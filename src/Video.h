#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <stack>

#include <opengl/context.hpp>
#include <opengl/texture.hpp>

class Video;

struct SDL_Surface;

class Video
{
public:
	bool init(int xres_, int yres_, bool fullscreen_, bool doAntiAliasing_);

	void close();

	void flip() const;
	void clearScreen() const;
	void set3D() const;
	void set3D_select() const;
	void set2D() const;
	void setTileMode() const;
	void resize(int w, int h);

	int xres() const
	{
		return _xres;
	}
	int yres() const
	{
		return _yres;
	}
	float ratio() const
	{
		return _ratio;
	}
	bool fullscreen() const
	{
		return _fullscreen;
	}
	bool doAntiAliasing() const
	{
		return _doAntiAliasing;
	}
	float fov() const
	{
		return _fov;
	}
	float nearclip() const
	{
		return _nearclip;
	}
	float farclip() const
	{
		return _farclip;
	}

	void doAntiAliasing(bool doAntiAliasing_)
	{
		_doAntiAliasing = doAntiAliasing_;
	}
	void fov(float fov_)
	{
		_fov = fov_;
	}
	void nearclip(float nearclip_)
	{
		_nearclip = nearclip_;
	}
	void farclip(float farclip_)
	{
		_farclip = farclip_;
	}

	void updateProjectionMatrix();

	/// is * supported:
	bool mSupportShaders;
	bool mSupportCompression;

private:
	int _xres;
	int _yres;
	float _ratio;

	float _fov;
	float _nearclip;
	float _farclip;

	bool _fullscreen;
	bool _doAntiAliasing;

	int _status;

	SDL_Surface* _primary;
};

#include "Manager.h" // ManagedItem

struct BLPHeader;

namespace OpenGL
{
	class Texture : public ManagedItem, public opengl::texture
	{
	public:
		Texture();

		void loadFromBLP(const std::string& filename);
		void loadFromUncompressedData(BLPHeader* lHeader, char* lData);
		void loadFromCompressedData(BLPHeader* lHeader, char* lData);

		const std::string& filename();

	private:
		int _width;
		int _height;
    std::string _filename;
	};

	typedef GLuint Shader;
	typedef GLuint Light;
}

extern Video video;

//bool isExtensionSupported(const char *search);

#endif
