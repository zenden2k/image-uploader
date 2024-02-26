#ifndef LOGLISTITEMDELEGATE_H
#define LOGLISTITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QObject>
#include <QWidget>

class LogListItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit LogListItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;
};

#endif // LOGLISTITEMDELEGATE_H
