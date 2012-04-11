// DBCEditor.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>
#include "DBCEditor.h"
#include <noggit/DBCTableModel.h>
#include <QMenu>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>



DBCEditor::DBCEditor(const QString& filename, QWidget *parent)
  : QWidget(parent)
  , view (new QTableView(this))
  , layout (new QVBoxLayout (this))
  , model (new DBCTableModel(filename))
{

    QPushButton *save = new QPushButton(tr("Save"),this);
    view->setEditTriggers(QAbstractItemView::DoubleClicked);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setModel(model);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(view);
    layout->addWidget(save);

    this->setLayout(layout);

    connect(view, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(showContextMenu(const QPoint &)));
    connect(save,SIGNAL(clicked()),model,SLOT(save()));
}

void DBCEditor::showContextMenu(const QPoint &point)
{
    QModelIndex index = view->indexAt(point);
    QList<QAction *> actions;
    if (index.isValid())
    {
        view->setCurrentIndex(index);
        columnToEdit = view->currentIndex().column();

        editColumn = new QAction(tr("Edit Column"),this);
        editColumn->setStatusTip(tr("Edit Column Settings"));
        connect(editColumn, SIGNAL(triggered()), this, SLOT(editColumnSettings()));
        actions.append(editColumn);

        if (actions.count() > 0)
            QMenu::exec(actions, view->mapToGlobal(point));
    }
}

void DBCEditor::editColumnSettings()
{
    columnEditor *edit = new columnEditor(columnToEdit, this);
    if(edit->exec() == QDialog::Accepted)
    {
        model->getSettings()->setValue(QString("column%1/name").arg(columnToEdit),edit->name->text());
        model->getSettings()->setValue(QString("column%1/type").arg(columnToEdit),edit->type->text());
        model->settingsChanged(columnToEdit,columnToEdit);
    }
}



columnEditor::columnEditor(int column, DBCEditor *parent)
    : QDialog(parent)
{
    QGridLayout *layout = new QGridLayout();

    QLabel *nameLabel = new QLabel(tr("Name"),this);
    name = new QLineEdit(parent->model->getSettings()->value(QString("column%1/name").arg(column),QString("column%1").arg(column)).toString(),this);
    QLabel *typeLabel = new QLabel(tr("Type"),this);
    type = new QLineEdit(parent->model->getSettings()->value(QString("column%1/type").arg(column),"int").toString(),this);
    QPushButton *ok = new QPushButton("&Ok",this);
    QPushButton *canel = new QPushButton("&Canel",this);

    ok->setDefault(true);
    layout->addWidget(nameLabel,0,0);
    layout->addWidget(name,1,0);
    layout->addWidget(typeLabel,3,0);
    layout->addWidget(type,4,0);
    layout->addWidget(canel,5,0);
    layout->addWidget(ok,5,1);
    this->setLayout(layout);
    connect(ok,SIGNAL(clicked()),this,SLOT(accept()));
    connect(canel,SIGNAL(clicked()),this,SLOT(reject()));
}


