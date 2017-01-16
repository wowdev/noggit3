// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Window.h>

#include <vector>
#include <string>

#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/Video.h>
#include <opengl/scoped.hpp>

UIWindow::UIWindow(float xPos, float yPos, float w, float h)
	: UIFrame(xPos, yPos, w, h)
	, texture(TextureManager::newTexture("interface\\tooltips\\ui-tooltip-border.blp"))
	, _textureFilename("interface\\tooltips\\ui-tooltip-border.blp")
{
}

UIWindow::UIWindow(float xPos, float yPos, float w, float h, const std::string& pTexture)
	: UIFrame(xPos, yPos, w, h)
	, texture(TextureManager::newTexture(pTexture))
	, _textureFilename(pTexture)
{
}

UIWindow::~UIWindow()
{
	TextureManager::delbyname(_textureFilename);
}

UIFrame::Ptr UIWindow::processLeftClick(float mx, float my)
{
	UIFrame::Ptr lTemp(UIFrame::processLeftClick(mx, my));
	if (lTemp)
	{
		return lTemp;
	}
	return this;
}



void UIWindow::render() const
{
  opengl::scoped::matrix_pusher const matrix;
	gl.translatef(x(), y(), 0.0f);

	gl.color4f(0.2f, 0.2f, 0.2f, 0.8f);
	gl.begin(GL_TRIANGLE_STRIP);
	gl.vertex2f(0.0f, 0.0f);
	gl.vertex2f(width(), 0.0f);
	gl.vertex2f(0.0f, height());
	gl.vertex2f(width(), height());
	gl.end();

	renderChildren();

	gl.color3f(1.0f, 1.0f, 1.0f);

	opengl::texture::set_active_texture();
	opengl::texture::enable_texture();

	texture->bind();

	//Draw Bottom left Corner First
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.75f, 1.0f);
	gl.vertex2f(-3.0f, height() + 3.0f);
	gl.texCoord2f(0.875f, 1.0f);
	gl.vertex2f(13.0f, height() + 3.0f);
	gl.texCoord2f(0.75f, 0.0f);
	gl.vertex2f(-3.0f, height() - 13.0f);
	gl.texCoord2f(0.875f, 0.0f);
	gl.vertex2f(13.0f, height() - 13.0f);
	gl.end();

	//Draw Bottom Right Corner
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.875f, 1.0f);
	gl.vertex2f(width() - 13.0f, height() + 3.0f);
	gl.texCoord2f(1.0f, 1.0f);
	gl.vertex2f(width() + 3.0f, height() + 3.0f);
	gl.texCoord2f(0.875f, 0.0f);
	gl.vertex2f(width() - 13.0f, height() - 13.0f);
	gl.texCoord2f(1.0f, 0.0f);
	gl.vertex2f(width() + 3.0f, height() - 13.0f);
	gl.end();

	//Draw Top Left Corner

	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.5f, 1.0f);
	gl.vertex2f(-3.0f, 13.0f);
	gl.texCoord2f(0.625f, 1.0f);
	gl.vertex2f(13.0f, 13.0f);
	gl.texCoord2f(0.5f, 0.0f);
	gl.vertex2f(-3.0f, -3.0f);
	gl.texCoord2f(0.625f, 0.0f);
	gl.vertex2f(13.0f, -3.0f);
	gl.end();

	//Draw Top Right Corner
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.625f, 1.0f);
	gl.vertex2f(width() - 13.0f, 13.0f);
	gl.texCoord2f(0.75f, 1.0f);
	gl.vertex2f(width() + 3.0f, 13.0f);
	gl.texCoord2f(0.625f, 0.0f);
	gl.vertex2f(width() - 13.0f, -3.0f);
	gl.texCoord2f(0.75f, 0.0f);
	gl.vertex2f(width() + 3.0f, -3.0f);
	gl.end();

	//Draw Left Side
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.0f, 1.0f);
	gl.vertex2f(-3.0f, height() - 13.0f);
	gl.texCoord2f(0.125f, 1.0f);
	gl.vertex2f(13.0f, height() - 13.0f);
	gl.texCoord2f(0.0f, 0.0f);
	gl.vertex2f(-3.0f, 13.0f);
	gl.texCoord2f(0.125f, 0.0f);
	gl.vertex2f(13, 13.0f);
	gl.end();

	//Draw Right Side
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.125f, 1.0f);
	gl.vertex2f(width() - 13.0f, height() - 13.0f);
	gl.texCoord2f(0.25f, 1.0f);
	gl.vertex2f(width() + 3.0f, height() - 13.0f);
	gl.texCoord2f(0.125f, 0.0f);
	gl.vertex2f(width() - 13.0f, 13.0f);
	gl.texCoord2f(0.25f, 0.0f);
	gl.vertex2f(width() + 3.0f, 13.0f);
	gl.end();

	//Draw Top Side
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.5f, 1.0f);
	gl.vertex2f(13.0f, height() + 3.0f);
	gl.texCoord2f(0.5f, 0.0f);
	gl.vertex2f(width() - 13.0f, height() + 3.0f);
	gl.texCoord2f(0.375f, 1.0f);
	gl.vertex2f(13, height() - 13.0f);
	gl.texCoord2f(0.375f, 0.0f);
	gl.vertex2f(width() - 13.0f, height() - 13.0f);
	gl.end();

	//Draw Bottom Side
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.375f, 1.0f);
	gl.vertex2f(13.0f, 13.0f);
	gl.texCoord2f(0.375f, 0.0f);
	gl.vertex2f(width() - 13.0f, 13.0f);
	gl.texCoord2f(0.25f, 1.0f);
	gl.vertex2f(13.0f, -3.0f);
	gl.texCoord2f(0.25f, 0.0f);
	gl.vertex2f(width() - 13.0f, -3.0f);
	gl.end();

	opengl::texture::disable_texture();
}
