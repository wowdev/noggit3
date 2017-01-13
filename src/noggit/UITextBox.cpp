// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "UITextBox.h"

#include <SDL.h>
#include <utf8.h>

#include <string>

#include "application.h" // arial12
#include "TextureManager.h" // TextureManager, Texture
#include "UIText.h"
#include "UITexture.h"
#include "Video.h"
#include <opengl/scoped.hpp>

//! \todo Handle Selection, Handle Clipboard ( CTRL + C / CTRL + V / CTRL + X ), Handle the Backspace staying down. Details, but better like that.

static const std::string texture("Interface\\Common\\Common-Input-Border.blp");
static const std::string textureFocused("Interface\\Common\\Common-Input-Border.blp");

UITextBox::UITextBox(float xPos, float yPos, float w, float h)
	: UIFrame(xPos, yPos, w, h)
	, _texture(TextureManager::newTexture(texture))
	, _textureFocused(TextureManager::newTexture(textureFocused))
	, _focus(false)
	, _uiText(new UIText(8.0f, 2.5f, app.getArial16(), eJustifyLeft))
	, _value("")
	, _enterFunction(NULL)
	, _updateFunction(NULL)
{
}

UITextBox::UITextBox(float xPos, float yPos, float w, float h, TriggerFunction enterFunction)
	: UIFrame(xPos, yPos, w, h)
	, _texture(TextureManager::newTexture(texture))
	, _textureFocused(TextureManager::newTexture(textureFocused))
	, _focus(false)
	, _uiText(new UIText(8.0f, 2.5f, app.getArial16(), eJustifyLeft))
	, _value("")
	, _enterFunction(enterFunction)
	, _updateFunction(NULL)
{
}

UITextBox::UITextBox(float xPos, float yPos, float w, float h, const freetype::font_data& pFont)
  : UIFrame(xPos, yPos, w, h)
  , _texture(TextureManager::newTexture(texture))
  , _textureFocused(TextureManager::newTexture(textureFocused))
  , _focus(false)
  , _uiText(new UIText(8.0f, 2.5f, pFont, eJustifyLeft))
  , _value("")
  , _enterFunction(NULL)
  , _updateFunction(NULL)
{
}

UITextBox::UITextBox(float xPos, float yPos, float w, float h, const freetype::font_data& pFont, TriggerFunction enterFunction)
  : UIFrame(xPos, yPos, w, h)
  , _texture(TextureManager::newTexture(texture))
  , _textureFocused(TextureManager::newTexture(textureFocused))
  , _focus(false)
  , _uiText(new UIText(8.0f, 2.5f, pFont, eJustifyLeft))
  , _value("")
  , _enterFunction(enterFunction)
  , _updateFunction(NULL)
{
}

UITextBox::~UITextBox()
{
	TextureManager::delbyname(texture);
	TextureManager::delbyname(textureFocused);
}

void UITextBox::render() const
{
  opengl::scoped::matrix_pusher const matrix;
	gl.translatef(x(), y(), 0.0f);

	gl.color3f(1.0f, 1.0f, 1.0f);

	opengl::texture::set_active_texture();
	opengl::texture::enable_texture();

	if (_focus)
		_textureFocused->bind();
	else
		_texture->bind();

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

	_uiText->render();
}

UIFrame::Ptr UITextBox::processLeftClick(float /*mx*/, float /*my*/)
{
	_focus = !_focus;
	return this;
}

void UITextBox::value(const std::string& value_)
{
	_value = value_;
	_uiText->setText(value_);
	if (_updateFunction)
	{
		_updateFunction(this, _value);
	}
}

const std::string& UITextBox::value() const
{
	return _value;
}

bool UITextBox::KeyBoardEvent(SDL_KeyboardEvent* e)
{
	if (!_focus || e->type != SDL_KEYDOWN)
	{
		return false;
	}

	if (e->keysym.sym == SDLK_BACKSPACE && !_value.empty())
	{
		const char* firstBeforeEnd(_value.c_str() + _value.length());
		utf8::prior(firstBeforeEnd, _value.c_str());
		_value.erase(firstBeforeEnd - _value.c_str());
	}
	else if (e->keysym.unicode > 31)
	{
		utf8::append(e->keysym.unicode, std::back_inserter(_value));
	}
	else if (e->keysym.sym == SDLK_RETURN)
	{
		_focus = false;
		if (_enterFunction)
		{
			_enterFunction(this, _value);
		}
	}
	else
	{
		_focus = false;
	}

	_uiText->setText(_value);

	return true;
}

void UITextBox::clear()
{
	value("");
}
