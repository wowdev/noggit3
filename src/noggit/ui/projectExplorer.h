#ifndef PROJECTEXPLORER_H
#define PROJECTEXPLORER_H

#include <QWidget>

class projectExplorer : public QWidget
{
    Q_OBJECT
public:
    explicit projectExplorer(const QString& projectPath, QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // PROJECTEXPLORER_H
