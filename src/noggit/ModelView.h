// ModelView.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <noggit/ModelManager.h>

#include <opengl/types.h>
#include <QGLWidget>
#include <QSettings>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTime>

class Model;

class ModelView : public QGLWidget
{

public:
    ModelView(QGLWidget *shared, QWidget *parent = 0);
    virtual QSize sizeHint() const;
    void changeModel(QString filename);

private:
    int frames;
    bool _draw_loading;
    QTime _run_time;
    qreal _last_update;
    noggit::scoped_model_reference theModel;
    GLfloat distance;
    GLfloat xRot;
    GLfloat yRot;
    QPoint lastPos;



protected:
    void initializeGL();
    void paintGL();
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void resizeGL (int width, int height);
    void wheelEvent(QWheelEvent *event);
    void timerEvent (QTimerEvent*);

};

#endif // MODELVIEW_H
