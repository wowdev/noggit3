#include "UITexture.h"

#include <string>

#include "TextureManager.h" // TextureManager
#include "Video.h" // Texture

UITexture::UITexture(float xPos, float yPos, float w, float h, const std::string& tex)
	: UIFrame(xPos, yPos, w, h)
	, texture(TextureManager::newTexture(tex))
	, _textureFilename(tex)
	, highlight(false)
	, clickFunc(NULL)
	, id(0)
{
}

UITexture::~UITexture()
{
	if (texture)
	{
		TextureManager::delbyname(_textureFilename);
		texture = NULL;
	}
}

void UITexture::setTexture(OpenGL::Texture* tex)
{
	//! \todo Free current texture.
	//! \todo New reference?
	texture = tex;
}

void UITexture::setTexture(const std::string& textureFilename)
{
	if (texture)
	{
		TextureManager::delbyname(_textureFilename);
		texture = NULL;
	}
	_textureFilename = textureFilename;
	texture = TextureManager::newTexture(textureFilename);
}

OpenGL::Texture* UITexture::getTexture()
{
	return texture;
}

void UITexture::render() const
{
	gl.pushMatrix();
	gl.translatef(x(), y(), 0.0f);

	gl.color3f(1.0f, 1.0f, 1.0f);

	opengl::texture::set_active_texture();
	opengl::texture::enable_texture();

	texture->bind();

	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.0f, 0.0f);
	gl.vertex2f(0.0f, 0.0f);
	gl.texCoord2f(1.0f, 0.0f);
	gl.vertex2f(width(), 0.0f);
	gl.texCoord2f(0.0f, 1.0f);
	gl.vertex2f(0.0f, height());
	gl.texCoord2f(1.0f, 1.0f);
	gl.vertex2f(width(), height());
	gl.end();

	opengl::texture::disable_texture();

	if (highlight)
	{
		gl.color3f(1.0f, 0.0f, 0.0f);
		gl.begin(GL_LINE_LOOP);
		gl.vertex2f(-1.0f, 0.0f);
		gl.vertex2f(width(), 0.0f);
		gl.vertex2f(width(), height());
		gl.vertex2f(-1.0f, height());
		gl.end();
	}

	gl.popMatrix();
}

UIFrame *UITexture::processLeftClick(float /*mx*/, float /*my*/)
{
	if (clickFunc)
	{
		clickFunc(this, id);
		return this;
	}
	return 0;
}

void UITexture::setClickFunc(void(*f)(UIFrame *, int), int num)
{
	clickFunc = f;
	id = num;
}
