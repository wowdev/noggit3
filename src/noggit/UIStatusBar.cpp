#include "UIStatusBar.h"

#include <vector>
#include <string>

#include "application.h" // app.getArial16()
#include "UIText.h"
#include "Video.h"
#include <opengl/scoped.hpp>

UIStatusBar::UIStatusBar(float xPos, float yPos, float w, float h)
	: UIWindow(xPos, yPos, w, h)
	, leftInfo(new UIText(8.0f, 7.0f, "", app.getArial16(), eJustifyLeft))
	, rightInfo(new UIText(width() - 8.0f, 7.0f, "", app.getArial16(), eJustifyRight))
{
	addChild(leftInfo);
	addChild(rightInfo);
}

void UIStatusBar::render() const
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

	UIFrame::renderChildren();

	gl.color3f(0.7f, 0.7f, 0.7f);

	opengl::texture::set_active_texture();
	opengl::texture::enable_texture();

	texture->bind();

	//Draw Top Side
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.375f, 1.0f);
	gl.vertex2f(0.0f, 13.0f);
	gl.texCoord2f(0.375f, 0.0f);
	gl.vertex2f(width(), 13.0f);
	gl.texCoord2f(0.25f, 1.0f);
	gl.vertex2f(0.0f, -3.0f);
	gl.texCoord2f(0.25f, 0.0f);
	gl.vertex2f(width(), -3.0f);
	gl.end();

	opengl::texture::disable_texture();
}

void UIStatusBar::resize()
{
	y(video.yres() - 30.0f);
	width((const float)video.xres());
	rightInfo->x(width() - 8.0f);
}

void UIStatusBar::setLeftInfo(const std::string& pText)
{
	leftInfo->setText(pText);
}

void UIStatusBar::setRightInfo(const std::string& pText)
{
	rightInfo->setText(pText);
}
