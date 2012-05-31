#include "textureselecter.h"

#include "noggit/application.h"
#include "noggit/TextureManager.h"

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

textureSelecter::textureSelecter(QGLWidget *shared, QWidget *parent) :
    QWidget(parent)
  , sharedWidget(shared)
{
    setMinimumSize(850,600);

    while(!app().archive_manager().all_finished_loading()) app().archive_manager().all_finish_loading();

    QVBoxLayout *layout = new QVBoxLayout(this);

    QScrollArea *scroll = new QScrollArea();
    QWidget *scrollWidget = new QWidget(NULL);
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);

    QStringList listfile = app().archive_manager().listfile().filter (QRegExp (".(_s.blp)", Qt::CaseInsensitive));
    QMap<QString, QStringList> arealist;


    for(QStringList::iterator it = listfile.begin(); it != listfile.end(); ++it)
    {
        QString area = it->section(QRegExp("(\\\\|/)"),1,1);
        if(!arealist.contains(area))
            arealist.insert(area,QStringList());
        arealist[area].append(*it);

    }

    for(QMap<QString, QStringList>::iterator it = arealist.begin(); it != arealist.end(); ++it)
    {
        QCheckBox *check = new  QCheckBox(it.key(),scrollWidget);
        check->setFocusPolicy(Qt::NoFocus);
        check->setStyleSheet("QCheckBox::indicator:unchecked {image: url(rightarrow-icon);}"
                             "QCheckBox::indicator:checked {image: url(downarrow-icon);}");
        textureView *view   = new textureView(it.value(), sharedWidget, scrollWidget);
        view->hide();


        connect(check,SIGNAL(toggled(bool)),view,SLOT(setVisible(bool)));

        scrollLayout->addWidget(check);
        scrollLayout->addWidget(view);
    }

    scrollWidget->setMinimumSize(770,500);
    scrollWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    scrollLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    scroll->setWidgetResizable(true);
    scroll->setMinimumSize(780,500);
    scroll->setWidget(scrollWidget);
    scroll->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    layout->addWidget(scroll);
}

textureItem::textureItem(QString texturename) :
    QGraphicsItem()
  , blptexture(TextureManager::newTexture(texturename.toStdString()))
{
    this->setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF textureItem::boundingRect() const
{
    return QRectF(QPointF(0,0),QSizeF(WIDTH,HEIGHT));
}

void textureItem::paint(QPainter */*painter*/, const QStyleOptionGraphicsItem *option, QWidget */*widget*/)
{
    glColor3f( 1.0f, 1.0f, 1.0f );
    opengl::texture::enable_texture (0);

    blptexture->bind();

    glBegin (GL_TRIANGLE_FAN);
    glTexCoord2f (0.0f, 0.0f);
    glVertex2f (pos().x(), pos().y());
    glTexCoord2f (1.0f, 0.0f);
    glVertex2f (pos().x()+option->rect.width()-5, pos().y());
    glTexCoord2f (1.0f, 1.0f);
    glVertex2f (pos().x()+option->rect.width()-5, pos().y()+option->rect.height()-5);
    glTexCoord2f (0.0f, 1.0f);
    glVertex2f (pos().x(), pos().y()+option->rect.height()-5);
    glEnd();

    opengl::texture::disable_texture (0);

    if( isSelected() )
    {
        glColor3f( 0.0f, 0.0f, 0.0f );
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(3);
        glBegin( GL_LINE_LOOP );
        glVertex2f(pos().x(),pos().y());
        glVertex2f( pos().x()+option->rect.width()-5, pos().y() );
        glVertex2f( pos().x()+option->rect.width()-5, pos().y()+option->rect.height()-5);
        glVertex2f( pos().x(), pos().y()+option->rect.height()-5);
        glEnd();
    }

}

textureScene::textureScene(QStringList textures, QRectF rect, QWidget *parent) :
    QGraphicsScene(rect,parent)
  , textureList(textures)
  , rows(0)
{

    for(int i=0; i < textureList.count(); ++i)
        addItem(new textureItem(textureList.at(i)));

    connect(this,SIGNAL(sceneRectChanged(QRectF)),this,SLOT(resized(QRectF)));
}

void textureScene::resized(QRectF /*rect*/)
{
    rows = 1;
    columns = this->sceneRect().width() / WIDTH;
    int verticalOffset = (width() - columns*WIDTH) / 2;
    for(int i=0; i < textureList.count(); ++i)
    {
        int x = (i-(rows-1)*columns)*WIDTH;
        int y = HEIGHT*(rows-1);
        items().at(i)->setPos(x+verticalOffset, y);
        if(i == columns*(rows)-1)
            rows++;
    }
}

void textureScene::drawBackground(QPainter *painter, const QRectF &)
{
    if (painter->paintEngine()->type() != QPaintEngine::OpenGL && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
        return;
    }

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0.0f, width(), height(), 0.0f, 1.0f, -1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(this->palette().background().color().redF(), this->palette().background().color().greenF(), this->palette().background().color().blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

textureView::textureView(QStringList textures, QGLWidget *shared, QWidget *parent) :
    QGraphicsView(parent)
{
    setViewport(new QGLWidget(parent,shared));
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setScene(new textureScene(textures,this->rect()));
    num = textures.size();
    setMinimumSize(QSize(770, ceil((float)num / 5) * HEIGHT + 20));
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setFrameShape(QFrame::NoFrame);
    setContentsMargins(0, 0, 0, 0);
}

QSize textureView::sizeHint() const
{
    return QSize(770, ceil((float)num / ((float)this->width() / (float)WIDTH)) * HEIGHT + 20);
}

void textureView::resizeEvent(QResizeEvent *event)
{
    if (scene())
    {
        int rows = ceil((float)num / ((float)event->size().width() / (float)WIDTH));
        int heightNeeded = (rows*HEIGHT);
        this->setMinimumHeight(heightNeeded+10);
        this->setMaximumHeight(heightNeeded+15);
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
