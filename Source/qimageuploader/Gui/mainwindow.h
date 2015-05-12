#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Core/UploadEngineList.h"

class UploadManager;
class UploadEngineManager;
class UploadTreeModel;
class ScriptsManager;

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

    void on_actionGrab_frames_triggered();
    void on_actionScreenshot_triggered();
    void on_actionAdd_files_triggered();

private:
    Ui::MainWindow *ui;
    UploadTreeModel* uploadTreeModel_;
    UploadManager* uploadManager_;
    CUploadEngineList engineList_;
    UploadEngineManager* uploadEngineManager_;
    ScriptsManager* scriptsManager_;
};

#endif // MAINWINDOW_H
