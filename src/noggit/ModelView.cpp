// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "ModelView.h"

#include <algorithm>
#include <ctime>
#include <string>

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Model.h>
#include <noggit/ModelManager.h>
#include <noggit/World.h>

#include <opengl/context.h>
#include <opengl/matrix.h>

static const qreal fov (45.0);

ModelView::ModelView(QWidget *parent) :
    QOpenGLWidget(parent)
  , theModel ("World\\Azeroth\\elwynn\\passivedoodads\\tree\\elwynnlog02.m2")
{
    _draw_loading = true;
    startTimer(40);
    _run_time.start();
}

void ModelView::initializeGL()
{
    makeCurrent();
    opengl::context::scoped_setter const _ (::gl, context());
    gl.clearColor (0.0f, 0.0f, 1.0f, 1.0f);

    gl.enableClientState (GL_VERTEX_ARRAY);
    gl.enableClientState (GL_NORMAL_ARRAY);
    gl.enableClientState (GL_TEXTURE_COORD_ARRAY);

    gl.enable(GL_DEPTH_TEST);
    gl.enable(GL_LIGHTING);
    gl.enable(GL_LIGHT0);
    gl.enable(GL_NORMALIZE);
    gl.enable(GL_COLOR_MATERIAL);
    gl.shadeModel(GL_SMOOTH);
    gl.enable(GL_TEXTURE_2D);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    gl.lightfv (GL_LIGHT0, GL_POSITION, lightPosition);

}

void ModelView::paintGL()
{
    makeCurrent();
    opengl::context::scoped_setter const _ (::gl, context());
    gl.matrixMode (GL_PROJECTION);
    gl.loadIdentity();

    const qreal ratio (width() / qreal (height()));
    opengl::matrix::perspective (fov, ratio, 2.0f, 600.0);

    gl.matrixMode (GL_MODELVIEW);
    gl.loadIdentity();

    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl.translatef(0.0, theModel->header.VertexBoxMin.y()/2, distance);
    gl.rotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    gl.rotatef(yRot / 16.0, 0.0, 1.0, 0.0);

    if(_draw_loading)
    {
      QPainter painter(this);
      painter.setPen(Qt::white);
      painter.setFont(QFont("Arial"));
      painter.drawText(width()/2, height()/2, QString("Loading..."));
      painter.end();
    }else
      //! \todo Have a local timer starting upon opening.
      theModel->draw(false, clock() / CLOCKS_PER_SEC);

}


//may not needed :P mapview seems to update ribbons and particles of the model here too
void ModelView::timerEvent (QTimerEvent*)
{
    if(_draw_loading && theModel->finished_loading()){
        distance = -(theModel->header.VertexBoxMax.z()*3);
        _draw_loading = false;
    }
    update();
}

QSize ModelView::sizeHint() const
{
    return QSize (320,320);
}

void ModelView::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void ModelView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        xRot = (xRot + 8 * dy);
        yRot = (yRot + 8 * dx);
        update();
    }
    lastPos = event->pos();
}

void ModelView::wheelEvent(QWheelEvent *event)
{
    event->accept();
    if (event->buttons() & Qt::LeftButton) {
        distance = distance + float(event->delta())/32;
        update();
    }
}

void ModelView::resizeGL (int width, int height)
{
  makeCurrent();
  opengl::context::scoped_setter const _ (::gl, context());
  gl.viewport (0.0f, 0.0f, width, height);
}

void ModelView::changeModel(QString filename)
{
    theModel = noggit::scoped_model_reference (filename.toStdString());
    _draw_loading = true;
    update();
}

