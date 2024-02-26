#ifndef LOGLISTMODEL_H
#define LOGLISTMODEL_H

#include <QAbstractListModel>
#include "Core/Logging/Logger.h"

class LogListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    void addItem(ILogger::LogMsgType MsgType, QString Sender, QString Msg, QString Info);
    explicit LogListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add data:
    //bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    struct ListItem {
        ILogger::LogMsgType type;
        QString sender, msg, info;
    };

private:
    QList<ListItem> items_;
};

#endif // LOGLISTMODEL_H
