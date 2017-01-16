// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifdef _WIN32
#define NOMINMAX
#endif // win32

#include <noggit/ui/Slider.h>

#include <iomanip>
#include <sstream>

#include <noggit/FreeType.h> // freetype::
#include <noggit/application.h> // app.getArial12()
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/Video.h>
#include <opengl/scoped.hpp>

UISlider::UISlider(float xPos, float yPos, float w, float s, float o)
	: UIFrame(xPos, yPos, w, 10.0f)
	, texture(TextureManager::newTexture("Interface\\Buttons\\UI-SliderBar-Border.blp"))
	, sliderTexture(TextureManager::newTexture("Interface\\Buttons\\UI-SliderBar-Button-Horizontal.blp"))
	, scale(s)
	, offset(o)
	, func(nullptr)
	, text("")
	, value(0.5f)
{
	clickable(true);
}

UISlider::~UISlider()
{
	TextureManager::delbyname("Interface\\Buttons\\UI-SliderBar-Border.blp");
	TextureManager::delbyname("Interface\\Buttons\\UI-SliderBar-Button-Horizontal.blp");
}

void UISlider::setFunc(void(*f)(float val))
{
	func = f;
}

void UISlider::setFunc(std::function<void(float)> pFunc)
{
	func = pFunc;
}

void UISlider::setValue(float f)
{
	value = std::min(1.0f, std::max(0.0f, f));
	if (func)
		func(value * scale + offset);
}

void UISlider::setText(const std::string& t)
{
	text = t;
}

UIFrame *UISlider::processLeftClick(float mx, float /*my*/)
{
	/*if((mx>(width*value-16))&&(mx<(width*value+16)))
	return this;
	return 0;*/

	value = std::min(1.0f, std::max(0.0f, mx / width()));
	if (func)
		func(value * scale + offset);

	return this;
}

bool UISlider::processLeftDrag(float mx, float /*my*/, float /*xChange*/, float /*yChange*/)
{
	//! \todo use change?
	float tx, ty;
	parent()->getOffset(&tx, &ty);
	mx -= tx;
	//my -= ty;

	value = std::min(1.0f, std::max(0.0f, mx / width()));
	if (func)
		func(value * scale + offset);

	return true;
}

void UISlider::render() const
{
	if (hidden())
		return;

	opengl::scoped::matrix_pusher const matrix;
	gl.translatef(x(), y(), 0.0f);

	gl.color3f(1.0f, 1.0f, 1.0f);

	std::stringstream temp;
	temp << text << std::fixed << std::setprecision(2) << (value * scale + offset);
	const std::string tempStr = temp.str();
	app.getArial12().shprint(width() / 2.0f - app.getArial12().width(tempStr) / 2.0f, -16.0f, tempStr);

	opengl::texture::set_active_texture();
	opengl::texture::enable_texture();

	texture->bind();

	const float height_plus = height() + 4.0f;
	const float height_minus = height() - 4.0f;
	const float width_plus = width() + 1.0f;
	const float width_minus = width() - 7.0f;

	//Draw Bottom left Corner First
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.75f, 1.0f);
	gl.vertex2f(-1.0f, height_plus);
	gl.texCoord2f(0.875f, 1.0f);
	gl.vertex2f(7.0f, height_plus);
	gl.texCoord2f(0.75f, 0.0f);
	gl.vertex2f(-1.0f, height_minus);
	gl.texCoord2f(0.875f, 0.0f);
	gl.vertex2f(7.0f, height_minus);
	gl.end();

	//Draw Bottom Right Corner
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.875f, 1.0f);
	gl.vertex2f(width_minus, height_plus);
	gl.texCoord2f(1.0f, 1.0f);
	gl.vertex2f(width_plus, height_plus);
	gl.texCoord2f(0.875f, 0.0f);
	gl.vertex2f(width_minus, height_minus);
	gl.texCoord2f(1.0f, 0.0f);
	gl.vertex2f(width_plus, height_minus);
	gl.end();

	//Draw Top Left Corner
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.5f, 1.0f);
	gl.vertex2f(-1.0f, 4.0f);
	gl.texCoord2f(0.625f, 1.0f);
	gl.vertex2f(7.0f, 4.0f);
	gl.texCoord2f(0.5f, 0.0f);
	gl.vertex2f(-1.0f, -4.0f);
	gl.texCoord2f(0.625f, 0.0f);
	gl.vertex2f(7.0f, -4.0f);
	gl.end();

	//Draw Top Right Corner
	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.625f, 1.0f);
	gl.vertex2f(width_minus, 4.0f);
	gl.texCoord2f(0.75f, 1.0f);
	gl.vertex2f(width_plus, 4.0f);
	gl.texCoord2f(0.625f, 0.0f);
	gl.vertex2f(width_minus, -4.0f);
	gl.texCoord2f(0.75f, 0.0f);
	gl.vertex2f(width_plus, -4.0f);
	gl.end();

	if (height() > 8.0f)
	{
		//Draw Left Side
		gl.begin(GL_TRIANGLE_STRIP);
		gl.texCoord2f(0.0f, 1.0f);
		gl.vertex2f(-1.0f, height_minus);
		gl.texCoord2f(0.125f, 1.0f);
		gl.vertex2f(7.0f, height_minus);
		gl.texCoord2f(0.0f, 0.0f);
		gl.vertex2f(-1.0f, 4.0f);
		gl.texCoord2f(0.125f, 0.0f);
		gl.vertex2f(7.0f, 4.0f);
		gl.end();

		//Draw Right Side
		gl.begin(GL_TRIANGLE_STRIP);
		gl.texCoord2f(0.125f, 1.0f);
		gl.vertex2f(width_minus, height_minus);
		gl.texCoord2f(0.25f, 1.0f);
		gl.vertex2f(width_plus, height_minus);
		gl.texCoord2f(0.125f, 0.0f);
		gl.vertex2f(width_minus, 4.0f);
		gl.texCoord2f(0.25f, 0.0f);
		gl.vertex2f(width_plus, 4.0f);
		gl.end();
	}

	if (width() > 14.0f)
	{
		//Draw Top Side
		gl.begin(GL_TRIANGLE_STRIP);
		gl.texCoord2f(0.5f, 1.0f);
		gl.vertex2f(7.0f, height_plus);
		gl.texCoord2f(0.5f, 0.0f);
		gl.vertex2f(width_minus, height_plus);
		gl.texCoord2f(0.375f, 1.0f);
		gl.vertex2f(7.0f, height_minus);
		gl.texCoord2f(0.375f, 0.0f);
		gl.vertex2f(width_minus, height_minus);
		gl.end();

		//Draw Bottom Side
		gl.begin(GL_TRIANGLE_STRIP);
		gl.texCoord2f(0.375f, 1.0f);
		gl.vertex2f(7.0f, 4.0f);
		gl.texCoord2f(0.375f, 0.0f);
		gl.vertex2f(width_minus, 4.0f);
		gl.texCoord2f(0.25f, 1.0f);
		gl.vertex2f(7.0f, -4.0f);
		gl.texCoord2f(0.25f, 0.0f);
		gl.vertex2f(width_minus, -4.0f);
		gl.end();
	}

	sliderTexture->bind();

	const float sliderpos_x = width() * value;
	const float sliderpos_y = height() / 2.0f;

	gl.begin(GL_TRIANGLE_STRIP);
	gl.texCoord2f(0.0f, 0.0f);
	gl.vertex2f(sliderpos_x - 16.0f, sliderpos_y - 16.0f);
	gl.texCoord2f(1.0f, 0.0f);
	gl.vertex2f(sliderpos_x + 16.0f, sliderpos_y - 16.0f);
	gl.texCoord2f(0.0f, 1.0f);
	gl.vertex2f(sliderpos_x - 16.0f, sliderpos_y + 16.0f);
	gl.texCoord2f(1.0f, 1.0f);
	gl.vertex2f(sliderpos_x + 16.0f, sliderpos_y + 16.0f);
	gl.end();

	opengl::texture::disable_texture();
}
