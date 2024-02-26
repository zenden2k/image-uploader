#include "LogListModel.h"

LogListModel::LogListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LogListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return items_.size();
}

QVariant LogListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QVariant var;
    var.setValue(items_[index.row()]);
    return var;
}

/*bool LogListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}*/

void LogListModel::addItem(ILogger::LogMsgType MsgType, QString Sender, QString Msg, QString Info) {
    beginInsertRows(QModelIndex(), items_.size(), items_.size());
    items_.push_back({MsgType, Sender, Msg, Info});
    // FIXME: Implement me!
    endInsertRows();
}
