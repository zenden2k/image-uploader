#ifndef UPLOADTREEVIEW_H
#define UPLOADTREEVIEW_H

#include <QTreeView>
#include <QWidget>
#include <QKeyEvent>

class UploadTreeView : public QTreeView
{
    Q_OBJECT
public:
    UploadTreeView(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent * event) override;
private:
    void copy();
};

#endif // UPLOADTREEVIEW_H
