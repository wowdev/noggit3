// DBCTableModel.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#include "DBCTableModel.h"
#include <QDir>

DBCTableModel::DBCTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

DBCTableModel::DBCTableModel(const QString& filename, QObject *parent)
    : QAbstractTableModel(parent)
    , dbcFile(new DBCFile(filename))
    , configFile(filename.mid(filename.lastIndexOf("\\")+1).append(".ini"))
{
    dbcFile->open();
    hasConfigFile = false;

    if(QFile::exists(configFile))
    {
        hasConfigFile = true;
        config = new QSettings(configFile,QSettings::IniFormat);
    }  
}

int DBCTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return dbcFile->getRecordCount();
}

int DBCTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return dbcFile->getFieldCount();
}

Qt::ItemFlags DBCTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool DBCTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return true;
}

QVariant DBCTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        DBCFile::Record record = dbcFile->getRecord(index.row());
        if(hasConfigFile)
        {
            QString value = QString("column%1/type").arg(index.column());
            QString type = config->value(value,"int").toString();

            if(type == "int")
                return record.getInt(index.column());
            else if(type == "uint")
                return record.getUInt(index.column());
            else if(type == "float")
                return record.getFloat(index.column());
            else if(type == "string")
            {
                if(role == Qt::DisplayRole)
                    return QString::fromUtf8 (record.getString(index.column()));
                return record.getUInt(index.column());
            }
            else if(type == "loc")
                return QString::fromUtf8 (record.getLocalizedString(index.column()));
            else
                return record.getInt(index.column());

        }
        else
            return QVariant(record.getUInt(index.column()));
    }
    return QVariant();
}

QVariant DBCTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if(orientation == Qt::Vertical)
            return QVariant(section);
        if(hasConfigFile)
            return config->value(QString("column%1/name").arg(section),QString("column%1").arg(section));
        else
            return QVariant(QString("column%1").arg(section));
    }
    return QVariant();
}

void DBCTableModel::settingsChanged(int first, int last)
{
    emit this->headerDataChanged(Qt::Horizontal,first,last);
}
