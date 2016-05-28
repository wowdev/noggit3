// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ModelManager.h>

#include <opengl/types.h>
#include <QOpenGLWidget>
#include <QSettings>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTime>

class Model;

class ModelView : public QOpenGLWidget
{

public:
    ModelView(QWidget *parent = 0);
    virtual QSize sizeHint() const;
    void changeModel(QString filename);

private:
    bool _draw_loading;
    QTime _run_time;
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
