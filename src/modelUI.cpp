#include "modelUI.h"
#include "video.h"
#include "model.h"

modelUI::modelUI(float xPos,float yPos,float w,float h) : frame( xPos, yPos, w, h )
{
}

void modelUI::render() const
{
	glMatrixMode(GL_PROJECTION);
	gluPerspective(45.0f, (GLfloat)video.xres/(GLfloat)video.yres, 1.0f, 1024.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(0, xres, yres, 0, -1.0, 1.0);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();



	glPushMatrix();

	static const float rot = 45.0f;

	glTranslatef( x + width / 2.0f, y + height / 2.0f, 0.0f );
	glRotatef( rot, 0.0f, 1.0f, 0.0f );
	glRotatef( 180, 1.0f, 0.0f, 0.0f );
	glScalef( 5.0f, 5.0f, 5.0f );

	glDisable(GL_FOG);


	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glColor4f(1,1,1,1);

	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	
	model->cam.setup( 0 );
	model->draw();

	video.set2D();
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);


	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);

	glPopMatrix();
}

void modelUI::setModel(Model* _setModel)
{
	model = _setModel;	
}
