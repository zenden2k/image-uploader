#include "UploadTreeView.h"

#include <algorithm>

#include <QApplication>
#include <QClipboard>

UploadTreeView::UploadTreeView(QWidget *parent): QTreeView(parent) {

}

void UploadTreeView::keyPressEvent(QKeyEvent * event)
{
    //if(event->matches(QKeySequence::Copy)) {
    //    copy();
    //} else {
        QTreeView::keyPressEvent(event);
    ///}
}

void UploadTreeView::copy() {
    /*QItemSelectionModel * selection = selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();

    if(indexes.size() < 1) {
        return;
    }
    std::sort(indexes.begin(), indexes.end());
    QModelIndex index = indexes.first();

    QString url = model()->data(index, Qt::UserRole).toString();
    QApplication::clipboard()->setText(url);*/
}
