// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/textureselecter.h>

#include <noggit/application.h>

#include <opengl/context.h>

#include <QResizeEvent>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QScrollBar>

#include <iostream>
#include <math.h>

namespace noggit
{
  namespace ui
  {
    textureSelecter::textureSelecter(QWidget* parent)
    : QWidget(parent)
    {
      setMinimumSize(850, 600);

      while(!app().archive_manager().all_finished_loading())
        app().archive_manager().all_finish_loading();

      QVBoxLayout* layout = new QVBoxLayout(this);

      QScrollArea* scroll = new QScrollArea();
      QWidget* scrollWidget = new QWidget(nullptr);
      QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

      QStringList listfile = app().archive_manager().listfile().filter (QRegExp (".(_s.blp)", Qt::CaseInsensitive));
      QMap<QString, QStringList> arealist;


      for(QStringList::iterator it = listfile.begin(); it != listfile.end(); ++it)
      {
        QString area = it->section(QRegExp("(\\\\|/)"), 1, 1);
        if(!arealist.contains(area))
          arealist.insert(area,QStringList());

        arealist[area].append(*it);
      }

      for(QMap<QString, QStringList>::iterator it = arealist.begin(); it != arealist.end(); ++it)
      {
        QCheckBox* check = new  QCheckBox(it.key(), scrollWidget);
        check->setFocusPolicy(Qt::NoFocus);
        check->setStyleSheet("QCheckBox::indicator:unchecked {image: url(rightarrow-icon);}"
                             "QCheckBox::indicator:checked {image: url(downarrow-icon);}");
        textureView* view  = new textureView(it.value(), scrollWidget);
        view->hide();

        connect(check, SIGNAL(toggled(bool)), view, SLOT(setVisible(bool)));

        scrollLayout->addWidget(check);
        scrollLayout->addWidget(view);
      }

      scrollWidget->setMinimumSize(770, 500);
      scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      scrollLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

      scroll->setWidgetResizable(true);
      scroll->setMinimumSize(780, 500);
      scroll->setWidget(scrollWidget);
      scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      layout->addWidget(scroll);
    }

    textureItem::textureItem(QString texturename)
    : QGraphicsItem()
    , blptexture(texturename.toStdString())
    {
      this->setFlag(QGraphicsItem::ItemIsSelectable);
    }

    QRectF textureItem::boundingRect() const
    {
      return QRectF(QPointF(0, 0), QSizeF(WIDTH, HEIGHT));
    }

    void textureItem::paint(QPainter* /*painter*/, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
    {
      gl.color3f(1.0f, 1.0f, 1.0f);
      opengl::texture::enable_texture (0);

      blptexture->bind();

      gl.begin(GL_TRIANGLE_FAN);
      gl.texCoord2f(0.0f, 0.0f);
      gl.vertex2f(pos().x(), pos().y());
      gl.texCoord2f(1.0f, 0.0f);
      gl.vertex2f(pos().x() + option->rect.width() - 5, pos().y());
      gl.texCoord2f(1.0f, 1.0f);
      gl.vertex2f(pos().x() + option->rect.width() - 5, pos().y() + option->rect.height() - 5);
      gl.texCoord2f(0.0f, 1.0f);
      gl.vertex2f(pos().x(), pos().y() + option->rect.height() - 5);
      gl.end();

      opengl::texture::disable_texture (0);

      if(isSelected())
      {
        gl.color3f(0.0f, 0.0f, 0.0f);
        gl.enable(GL_LINE_SMOOTH);
        gl.lineWidth(3);
        gl.begin(GL_LINE_LOOP);
        gl.vertex2f(pos().x(), pos().y());
        gl.vertex2f(pos().x() + option->rect.width() - 5, pos().y());
        gl.vertex2f(pos().x() + option->rect.width() - 5, pos().y()+option->rect.height() - 5);
        gl.vertex2f(pos().x(), pos().y() + option->rect.height() - 5);
        gl.end();
      }
    }

    textureScene::textureScene(QStringList textures, QRectF rect, QWidget* parent)
    : QGraphicsScene(rect,parent)
    , textureList(textures)
    , rows(0)
    {
      for(int i = 0; i < textureList.count(); ++i)
        addItem(new textureItem(textureList.at(i)));

      connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(resized(QRectF)));
    }

    void textureScene::resized(QRectF /*rect*/)
    {
      rows = 1;
      columns = this->sceneRect().width() / WIDTH;
      int verticalOffset = (width() - columns * WIDTH) / 2;

      for(int i = 0; i < textureList.count(); ++i)
      {
        int x = (i - (rows - 1) * columns) * WIDTH;
        int y = HEIGHT * (rows - 1);
        items().at(i)->setPos(x+verticalOffset, y);
        if(i == columns * (rows) - 1)
          rows++;
      }
    }

    void textureScene::drawBackground(QPainter* painter, const QRectF&)
    {
      if (painter->paintEngine()->type() != QPaintEngine::OpenGL && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
      {
        qWarning("OpenGLScene: drawBackground needs a QOpenGLWidget to be set as viewport on the graphics view");
        return;
      }

      gl.matrixMode(GL_PROJECTION);
      gl.loadIdentity();
      gl.ortho(0.0f, width(), height(), 0.0f, 1.0f, -1.0f);
      gl.matrixMode(GL_MODELVIEW);
      gl.loadIdentity();

      gl.clearColor(this->palette().background().color().redF(), this->palette().background().color().greenF(), this->palette().background().color().blueF(), 1.0f);
      gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    textureView::textureView(QStringList textures, QWidget *parent)
    : QGraphicsView(parent)
    {
      setViewport (new QOpenGLWidget(parent));
      setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
      setScene(new textureScene(textures, this->rect()));
      num = textures.size();
      setMinimumSize(QSize(770, std::ceil((float)num / 5) * HEIGHT + 20));
      setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
      setFrameShape(QFrame::NoFrame);
      setContentsMargins(0, 0, 0, 0);
    }

    QSize textureView::sizeHint() const
    {
      return QSize(770, std::ceil((float)num / ((float)this->width() / (float)WIDTH)) * HEIGHT + 20);
    }

    void textureView::resizeEvent(QResizeEvent* event)
    {
      if (scene())
      {
        int rows = std::ceil((float)num / ((float)event->size().width() / (float)WIDTH));
        int heightNeeded = (rows * HEIGHT);
        this->setMinimumHeight(heightNeeded + 10);
        this->setMaximumHeight(heightNeeded + 15);
        if(event->size().height() < heightNeeded)
        {
          event->ignore();
          return;
        }
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
      }
      QGraphicsView::resizeEvent(event);
    }
  }
}
