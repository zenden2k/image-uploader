#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Core/ProgramWindow.h"
#include "Core/UploadEngineList.h"

class UploadManager;
class UploadEngineManager;
class UploadTreeModel;
class ScriptsManager;
struct UploadObject;
class ServerSelectorWidget;
class LogWindow;
class QSystemTrayIcon;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public IProgramWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(CUploadEngineList*, LogWindow* logWindow, QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *, QEvent *);

    WindowHandle getHandle() override;
    WindowNativeHandle getNativeHandle() override;
    void setServersChanged(bool changed) override;
private slots:
    void updateView();

    void on_actionGrab_frames_triggered();
    void on_actionScreenshot_triggered();
    void on_actionAdd_files_triggered();
    void on_actionAboutProgram_triggered();
	void itemDoubleClicked(const QModelIndex &index);
	void onCustomContextMenu(const QPoint &point);
	void onShowLog();
protected:
	bool addFileToList(QString fileName);
	bool addMultipleFilesToList(QStringList fileNames);
	void uploadTaskToUploadObject(UploadTask* task, UploadObject& obj);
	void showCodeForIndex(const QModelIndex& index);
	void quitApp();
private:
    std::unique_ptr<Ui::MainWindow> ui;
    UploadTreeModel* uploadTreeModel_;
    std::unique_ptr<UploadManager> uploadManager_;
    std::unique_ptr<UploadEngineManager> uploadEngineManager_;
    std::unique_ptr<ScriptsManager> scriptsManager_;
    QSystemTrayIcon* systemTrayIcon_;
	ServerSelectorWidget* imageServerWidget, *fileServerWidget;
	LogWindow* logWindow_;
	CUploadEngineList* engineList_;
};

#endif // MAINWINDOW_H
