#include "Model.h"
#include "UIModel.h"
#include "Video.h"

#include "ModelManager.h"

UIModel::UIModel(float xPos, float yPos, float w, float h)
	: UIFrame(xPos, yPos, w, h)
	, model(boost::none)
{
	gl.genFramebuffers(1, &fbo);
	gl.genRenderbuffers(1, &depthBuffer);
	gl.genTextures(1, &modelTexture);

	gl.bindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	gl.renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width(), (GLsizei)height());

	opengl::texture::enable_texture (0);
	gl.bindTexture(GL_TEXTURE_2D, modelTexture);
	gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width(), (GLsizei)height(), 0, GL_RGBA, GL_FLOAT, NULL);

	gl.bindFramebuffer(GL_FRAMEBUFFER, fbo);
	gl.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexture, 0);
	gl.framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
	gl.bindRenderbuffer(GL_RENDERBUFFER, 0);
}

void UIModel::drawFBO() const
{
	gl.pushMatrix();
	gl.pushAttrib(GL_VIEWPORT_BIT);
	gl.viewport(0, 0, (GLsizei)width(), (GLsizei)height());

	gl.bindFramebuffer(GL_FRAMEBUFFER, fbo);

	gl.clearColor(1.0f, 1.0f, 1.0f, 0.0f);
	gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gl.matrixMode(GL_PROJECTION);
	gl.loadIdentity();
	gluPerspective(video.fov(), width() / height(), video.nearclip(), video.farclip());
	gl.matrixMode(GL_MODELVIEW);
	gl.loadIdentity();

	gluLookAt(0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);


	gl.pushMatrix();
	gl.translatef(0.0f, 0.0f, -50.0f);
	gl.rotatef(180.0f, 0.0f, 0.0f, 1.0f);

	opengl::texture::enable_texture (0);
	gl.enable(GL_NORMALIZE);

	model.get()->draw();
	gl.popMatrix();

	gl.popAttrib();
	gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
	gl.popMatrix();
}

void UIModel::drawTexture() const
{
	gl.pushMatrix();
	video.set2D();
	gl.popMatrix();

	gl.pushMatrix();
	gl.translatef(x(), y(), 0.0f);

	opengl::texture::enable_texture (0);
	gl.bindTexture(GL_TEXTURE_2D, modelTexture);

	gl.begin(GL_QUADS);
	gl.texCoord2f(0.0f, 0.0f);
	gl.vertex2f(0.0f, 0.0f);
	gl.texCoord2f(1.0f, 0.0f);
	gl.vertex2f(width(), 0.0f);
	gl.texCoord2f(1.0f, 1.0f);
	gl.vertex2f(width(), height());
	gl.texCoord2f(0.0f, 1.0f);
	gl.vertex2f(0.0f, height());
	gl.end();

	opengl::texture::disable_texture (0);
	gl.popMatrix();
}

void UIModel::render() const
{
	if (!model) return;

	drawFBO();
	drawTexture();
}

void UIModel::setModel(const std::string &name)
{
	model = scoped_model_reference (name);
}

//! \todo create class for framebuffers and implement this check
/*
GLenum fboStatus = gl.checkFramebufferStatus(GL_FRAMEBUFFER);
if(fboStatus != GL_FRAMEBUFFER_COMPLETE)
{
switch(fboStatus)
{
case GL_FRAMEBUFFER_UNDEFINED:
return;
case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
return;
case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
return;
case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
return;
case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
return;
case GL_FRAMEBUFFER_UNSUPPORTED:
return;
case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
return;
case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
return;
}
}
*/
