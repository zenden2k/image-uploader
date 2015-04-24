#ifndef FRAMEGRABBERDLG_H
#define FRAMEGRABBERDLG_H

#include <QDialog>
#include "Core/Video/AbstractImage.h"
namespace Ui {
class FrameGrabberDlg;
}

class FrameGrabberDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit FrameGrabberDlg(QWidget *parent = 0);
    ~FrameGrabberDlg();
    
    void frameGrabbed(const Utf8String&, int64_t, AbstractImage*);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
private:
    Ui::FrameGrabberDlg *ui;
};

#endif // FRAMEGRABBERDLG_H
