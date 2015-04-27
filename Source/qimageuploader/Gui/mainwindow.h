#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *, QEvent *);
private slots:
    void updateView();

    void on_pushButton_3_clicked();

    void on_actionGrab_frames_triggered();

    void on_actionScreenshot_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
