#ifndef FRAMEGRABBERDLG_H
#define FRAMEGRABBERDLG_H

#include <QDialog>
#include "Core/Video/AbstractImage.h"
class VideoGrabber;

namespace Ui {
class FrameGrabberDlg;
}

class FrameGrabberDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit FrameGrabberDlg(QString fileName, QWidget *parent = 0);
    ~FrameGrabberDlg();
    
    void frameGrabbed(const std::string&, int64_t, AbstractImage*);
private slots:
    void on_grabButton_clicked();

    void on_browseButton_clicked();
	void onStopButtonClicked();
protected:
	void onGrabFinished();
private:
    Ui::FrameGrabberDlg *ui;
	std::unique_ptr<VideoGrabber> grabber_;
};

#endif // FRAMEGRABBERDLG_H
