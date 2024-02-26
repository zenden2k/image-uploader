#include "LogListItemDelegate.h"

#include <QPainter>
#include <QPoint>

LogListItemDelegate::LogListItemDelegate(QObject *parent)
    : QAbstractItemDelegate{parent}
{

}

void LogListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setPen(Qt::black);
    painter->drawText(0, 20, "One");
    painter->setPen(Qt::blue);
    painter->drawText(0,50, QString("Hello world"));
}

QSize LogListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }
    return QSize(100,30);
}
