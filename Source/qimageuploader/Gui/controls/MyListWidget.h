#ifndef QIMAGEUPLOADER_GUI_CONTROLS_MYLISTWIDGET
#define QIMAGEUPLOADER_GUI_CONTROLS_MYLISTWIDGET

#include <QListWidget>
#include <QKeyEvent>

class MyListWidget : public QListWidget
{
    Q_OBJECT
    private:

    public:
    MyListWidget(QWidget* parent = 0);

    void keyPressEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent *event) override;

public slots:

};

#endif