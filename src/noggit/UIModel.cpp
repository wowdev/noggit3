// UIModel.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/UIModel.h>

#include <noggit/Model.h>

#include <opengl/context.hpp>
#include <opengl/matrix.h>

UIModel::UIModel( float xPos, float yPos, float w, float h )
: UIFrame( xPos, yPos, w, h )
, model( nullptr )
{
}

void UIModel::render() const
{
  //! \todo Fix, save matrixes before changing. or something.
 /* gl.matrixMode(GL_PROJECTION);
  opengl::matrix::perspective (45.0f, (GLfloat)video.xres()/(GLfloat)video.yres(), 1.0f, 1024.0f);
  gl.matrixMode(GL_MODELVIEW);
  gl.loadIdentity();

  //gl.matrixMode(GL_PROJECTION);
  //gl.loadIdentity();
  //gl.ortho(0, xres(), yres(), 0, -1.0, 1.0);
  //gl.matrixMode(GL_MODELVIEW);
  //gl.loadIdentity();

  gl.pushMatrix();

  static const float rot = 45.0f;

  gl.translatef( x() + width() / 2.0f, y() + height() / 2.0f, 0.0f );
  gl.rotatef( rot, 0.0f, 1.0f, 0.0f );
  gl.rotatef( 180, 1.0f, 0.0f, 0.0f );
  gl.scalef( 5.0f, 5.0f, 5.0f );

  gl.disable(GL_FOG);


  gl.enable(GL_COLOR_MATERIAL);
  gl.colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  gl.color4f(1,1,1,1);

  gl.disable(GL_CULL_FACE);
  gl.enable(GL_TEXTURE_2D);
  gl.enable(GL_LIGHTING);

  model->cam.setup( 0 );
  //! \todo This will crash instantly. This would need passing stuff from inside World into model, not passing world.
  model->draw (nullptr);

  video.set2D();
  */
  gl.enable(GL_BLEND);
  gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.disable(GL_DEPTH_TEST);
  gl.disable(GL_CULL_FACE);
  gl.disable(GL_LIGHTING);


  gl.color4f(1,1,1,1);

  gl.enable(GL_TEXTURE_2D);

  gl.popMatrix();
}

void UIModel::setModel( Model* _setModel )
{
  model = _setModel;
}
