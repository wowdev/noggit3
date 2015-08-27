#include "Model.h"
#include "UIModel.h"
#include "Video.h"

#include "ModelManager.h"

UIModel::UIModel(float xPos, float yPos, float w, float h)
	: UIFrame(xPos, yPos, w, h)
	, model(boost::none)
{
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &depthBuffer);
	glGenTextures(1, &modelTexture);

	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width(), (GLsizei)height());

	OpenGL::Texture::enableTexture(0);
	glBindTexture(GL_TEXTURE_2D, modelTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width(), (GLsizei)height(), 0, GL_RGBA, GL_FLOAT, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, modelTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void UIModel::drawFBO() const
{
	glPushMatrix();
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)width(), (GLsizei)height());

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(video.fov(), width() / height(), video.nearclip(), video.farclip());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);


	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -50.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);

	OpenGL::Texture::enableTexture(0);
	glEnable(GL_NORMALIZE);

	model.get()->draw();
	glPopMatrix();

	glPopAttrib();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopMatrix();
}

void UIModel::drawTexture() const
{
	glPushMatrix();
	video.set2D();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x(), y(), 0.0f);

	OpenGL::Texture::enableTexture(0);
	glBindTexture(GL_TEXTURE_2D, modelTexture);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(width(), 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(width(), height());
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, height());
	glEnd();

	CheckForGLError("UIModel::draw:: after quads");

	OpenGL::Texture::disableTexture(0);
	glPopMatrix();
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
GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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
