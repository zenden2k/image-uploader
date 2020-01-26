#ifndef QIMAGEUPLOADER_GUI_CONTROLS_PROGRESSRINGWIDGET
#define QIMAGEUPLOADER_GUI_CONTROLS_PROGRESSRINGWIDGET

#include <QWidget>

class ProgressRingWidget : public QWidget
{
    Q_OBJECT
private:

public:
    ProgressRingWidget(QWidget* parent = 0);

protected:
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

    double current_;

};

#endif