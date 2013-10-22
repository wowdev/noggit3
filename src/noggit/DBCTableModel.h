// DBCTableModel.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>


#ifndef DBCTABLEMODEL_H
#define DBCTABLEMODEL_H

#include <QString>
#include <QAbstractTableModel>
#include <noggit/DBCFile.h>
#include <QSettings>


class DBCTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DBCTableModel(QObject *parent=0);
    DBCTableModel(const QString& filename, QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    inline QSettings *getSettings()
    {
        if(!hasConfigFile)
        {
            config = new QSettings(configFile,QSettings::IniFormat);
            hasConfigFile = true;
        }
        return config;
    }


private:
    DBCFile *dbcFile;
    QSettings *config;
    bool hasConfigFile;
    const QString configFile;

public slots:
    void settingsChanged(int first,int last);
    void save();
};



#endif // DBCTABLEMODEL_H
