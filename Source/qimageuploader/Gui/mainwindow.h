#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Core/UploadEngineList.h"

class UploadManager;
class UploadEngineManager;
class UploadTreeModel;
class ScriptsManager;
struct ZUploadObject;
class ServerSelectorWidget;
class LogWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(CUploadEngineList*, LogWindow* logWindow, QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *, QEvent *);
private slots:
    void updateView();

    void on_actionGrab_frames_triggered();
    void on_actionScreenshot_triggered();
    void on_actionAdd_files_triggered();
	void itemDoubleClicked(const QModelIndex &index);
	void onCustomContextMenu(const QPoint &point);
	void onShowLog();
protected:
	bool addFileToList(QString fileName);
	bool addMultipleFilesToList(QStringList fileNames);
	void uploadTaskToUploadObject(UploadTask* task, ZUploadObject& obj);
	void showCodeForIndex(const QModelIndex& index);
private:
    std::unique_ptr<Ui::MainWindow> ui;
    UploadTreeModel* uploadTreeModel_;
    std::unique_ptr<UploadManager> uploadManager_;
    std::unique_ptr<UploadEngineManager> uploadEngineManager_;
    std::unique_ptr<ScriptsManager> scriptsManager_;
	ServerSelectorWidget* imageServerWidget, *fileServerWidget;
	LogWindow* logWindow_;
	CUploadEngineList* engineList_;
};

#endif // MAINWINDOW_H
