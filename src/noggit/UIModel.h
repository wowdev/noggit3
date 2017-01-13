// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include "ModelManager.h"
#include "UIFrame.h"

#include <boost/optional.hpp>

class Model;

class UIModel : public UIFrame
{
public:
	UIModel(float x, float y, float width, float height);

	void render() const;
	void setModel(const std::string &name);

private:
	boost::optional<scoped_model_reference> model;

	GLuint fbo;
	GLuint modelTexture;
	GLuint depthBuffer;

	void drawFBO() const;
	void drawTexture() const;
};
