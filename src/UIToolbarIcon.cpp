#include "UIToolbarIcon.h"

#include <string>

#include "TextureManager.h" // TextureManager, Texture
#include "Video.h" // gl*

UIToolbarIcon::UIToolbarIcon(float xPos, float yPos, const std::string& tex, const std::string& texd, const int& id, UIEventClassConstructorArguments)
	: UIFrame(xPos, yPos, 45.0f, 45.0f)
	, UIEventClassConstructorSuperCall()
	, texture(TextureManager::newTexture(tex))
	, textureSelected(TextureManager::newTexture(texd))
	, _textureFilename(tex)
	, _textureSelectedFilename(texd)
	, iconId(id)
	, selected(false)
{
}

UIToolbarIcon::~UIToolbarIcon()
{
	TextureManager::delbyname(_textureFilename);
	TextureManager::delbyname(_textureSelectedFilename);
}

UIFrame* UIToolbarIcon::processLeftClick(float /*mx*/, float /*my*/)
{
	UIEventEventHandlerCall(iconId);

	return this;
}

void UIToolbarIcon::render() const
{
	glPushMatrix();
	glTranslatef(x(), y(), 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);

	OpenGL::Texture::setActiveTexture();
	OpenGL::Texture::enableTexture();

	texture->bind();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(width(), 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, height());
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(width(), height());
	glEnd();

	OpenGL::Texture::disableTexture();

	if (selected)
	{
		static const float sizer = 18.0f;

		OpenGL::Texture::enableTexture();

		textureSelected->bind();

		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-sizer, -sizer);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(width() + sizer, -sizer);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-sizer, height() + sizer);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(width() + sizer, height() + sizer);
		glEnd();

		OpenGL::Texture::disableTexture();
	}

	glPopMatrix();
}
