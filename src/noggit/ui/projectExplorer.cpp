#include "projectExplorer.h"
#include <QFileSystemModel>
#include <QTreeView>
#include <QVBoxLayout>

projectExplorer::projectExplorer(const QString& projectPath, QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* layout  = new QVBoxLayout(this);
    QFileSystemModel *model = new QFileSystemModel();
    QTreeView *tree = new QTreeView(this);
    model->setRootPath(projectPath);
    tree->setModel(model);
    tree->setRootIndex(model->index(projectPath));
    layout->addWidget(tree);

}
