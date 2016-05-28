// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/blp_texture.h>
#include <noggit/TextureManager.h>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QDebug>
#include <QPainter>

#define WIDTH 150
#define HEIGHT 150

class QResizeEvent;

namespace noggit
{
  namespace ui
  {
    class textureSelecter : public QWidget
    {
      Q_OBJECT

    public:
      textureSelecter(QWidget *parent = 0);
    };

    class textureView : public QGraphicsView
    {
      Q_OBJECT

    public:
      textureView(QStringList textures, QWidget* parent = 0);
      void resizeEvent(QResizeEvent* event);
      QSize sizeHint() const;

    private:
      int num;
      int rows;
    };

    class textureScene : public QGraphicsScene
    {
      Q_OBJECT

    public:
      textureScene(QStringList textures, QRectF rect, QWidget* parent= 0);
      void drawBackground(QPainter* painter, const QRectF&);

    public slots:
      void resized(QRectF);

    private:
      QStringList textureList;
      int rows, columns;
    };

    class textureItem : public QGraphicsItem
    {
    public:
      textureItem(QString texturename);

      void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/);
      QRectF boundingRect() const;

    private:
      noggit::scoped_blp_texture_reference blptexture;
    };
  }
}
