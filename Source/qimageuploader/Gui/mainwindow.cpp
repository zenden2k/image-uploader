#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <QDebug>
#include <QFileDialog>
#include <QToolButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTemporaryFile>
#include <Gui/FrameGrabberDlg.h>
#include <Gui/RegionSelect.h>
#include "Gui/controls/ServerSelectorWidget.h"
#include "models/uploadtreemodel.h"
#include "Core/CommonDefs.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/ServiceLocator.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/AppParams.h"
#include "Core/Settings.h"
#include "Core/Network/NetworkClientFactory.h"
#include "ResultsWindow.h"
#include "Gui/LogWindow.h"
#include "Core/OutputCodeGenerator.h"


MainWindow::MainWindow(CUploadEngineList* engineList, LogWindow* logWindow, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    logWindow_(logWindow) {
    ui->setupUi(this);
    ServiceLocator::instance()->setEngineList(engineList);
    engineList_ = engineList;

    auto networkClientFactory = std::make_shared<NetworkClientFactory>();
    scriptsManager_ = std::make_unique<ScriptsManager>(networkClientFactory);
    IUploadErrorHandler* uploadErrorHandler = ServiceLocator::instance()->uploadErrorHandler();
    uploadEngineManager_ = std::make_unique<UploadEngineManager>(engineList, uploadErrorHandler, networkClientFactory);
    uploadManager_ = std::make_unique<UploadManager>(uploadEngineManager_.get(), engineList, scriptsManager_.get(), uploadErrorHandler,
                                       networkClientFactory);

    std::string scriptsDirectory = AppParams::instance()->dataDirectory() + "/Scripts/";
    uploadEngineManager_->setScriptsDirectory(scriptsDirectory);

    uploadTreeModel_ = new UploadTreeModel(this, uploadManager_.get());

    ui->treeView->setModel(uploadTreeModel_);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView, &QTreeView::doubleClicked, this, &MainWindow::itemDoubleClicked);

    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::onCustomContextMenu);

    ServerProfile imageProfile("directupload.net");
    imageServerWidget = new ServerSelectorWidget(uploadEngineManager_.get(), false, this);
    imageServerWidget->setTitle(tr("Server for images:"));
    imageServerWidget->setServerProfile(imageProfile);
    ui->verticalLayout->insertWidget(1, imageServerWidget);

    fileServerWidget = new ServerSelectorWidget(uploadEngineManager_.get(), false, this);
    fileServerWidget->setTitle(tr("Server for other file types:"));
    fileServerWidget->setServersMask(ServerSelectorWidget::smFileServers);
    fileServerWidget->updateServerList();
    ui->verticalLayout->insertWidget(2, fileServerWidget);

    connect(ui->showLogButton, &QPushButton::clicked, this, &MainWindow::onShowLog);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ChildRemoved) {
        // ui->listWidget->setFlow(QListWidget::LeftToRight);
    }
    return false;
}

MainWindow::~MainWindow() {
}

void MainWindow::updateView() {

}

void MainWindow::on_actionGrab_frames_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (fileName.length()) {
        FrameGrabberDlg dlg(fileName);
        if (dlg.exec() == QDialog::Accepted) {
            QStringList frames;
            dlg.getGrabbedFrames(frames);
            addMultipleFilesToList(frames);
        }
    }
}

void MainWindow::on_actionScreenshot_triggered() {
    std::unique_ptr<RegionSelect> selector;
    CScreenCaptureEngine eng;
    eng.setDelay(450);
    hide();
    eng.captureScreen();

    std::unique_ptr<CScreenshotRegion> r;
    selector.reset(new RegionSelect(nullptr, eng.capturedBitmap()));

    if (selector->exec() == QDialog::Accepted) {
        r.reset(selector->selectedRegion());
    } else {
        show();
        return;
    }

    if (!r) {
        r.reset(new CActiveWindowRegion());
    }

    eng.setSource(*eng.capturedBitmap());
    if (!eng.captureRegion(r.get())) {
        show();
        return;
    }

    QPixmap* screen = eng.capturedBitmap(); //QPixmap::grabWindow(QApplication::desktop()->winId());
    QTemporaryFile f(U2Q(AppParams::instance()->tempDirectory()) + "/screenshot_XXXXXX.png");
    f.setAutoRemove(false);
    QString uniqueFileName;
    if (f.open()) {
        uniqueFileName = f.fileName();
        f.close();
    }
    if (!uniqueFileName.isEmpty()) {
        if (screen->save(uniqueFileName)) {
            addFileToList(uniqueFileName);
        }
    }
    show();

}

void MainWindow::on_actionAdd_files_triggered() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this);
    if (!fileNames.isEmpty()) {
        addMultipleFilesToList(fileNames);
    }
}

bool MainWindow::addFileToList(QString fileName) {
    auto uploadSession = std::make_shared<UploadSession>();
    auto task = std::make_shared<FileUploadTask>(Q2U(fileName), IuCoreUtils::ExtractFileName(Q2U(fileName)));
    ServerProfile serverProfile = imageServerWidget->serverProfile();
    task->setServerProfile(serverProfile);
    uploadSession->addTask(task);
    uploadManager_->addSession(uploadSession);
    ui->treeView->expandAll();
    return true;
}

bool MainWindow::addMultipleFilesToList(QStringList fileNames) {
    if (fileNames.isEmpty()) {
        return false;
    }
    auto uploadSession = std::make_shared<UploadSession>();
    for (const auto& fileName : fileNames) {
        auto task = std::make_shared<FileUploadTask>(Q2U(fileName), IuCoreUtils::ExtractFileName(Q2U(fileName)));
        ServerProfile serverProfile = imageServerWidget->serverProfile();
        task->setServerProfile(serverProfile);
        uploadSession->addTask(task);
    }

    uploadManager_->addSession(uploadSession);
    return true;
}

void MainWindow::itemDoubleClicked(const QModelIndex& index) {
    showCodeForIndex(index);
}

void MainWindow::uploadTaskToUploadObject(UploadTask* task, ZUploadObject& obj) {

    /*HistoryItem hi;
    hi.localFilePath = fileTask->originalFileName();
    hi.serverName = fileTask->serverProfile().serverName();*/
    obj.serverName = task->serverProfile().serverName();
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);

    obj.localFilePath = fileTask ? fileTask->originalFileName() : std::string();

    UploadResult* uploadResult = task->uploadResult();
    obj.directUrl = uploadResult->directUrl;
    //obj.directUrlShortened = uploadResult->directUrlShortened;
    obj.thumbUrl = uploadResult->thumbUrl;
    obj.viewUrl = uploadResult->downloadUrl;
    //obj.viewUrlShortened = uploadResult->downloadUrlShortened;
    //obj.editUrl = uploadResult->editUrl;
    //obj.deleteUrl = uploadResult->deleteUrl;
    obj.displayFileName = fileTask ? fileTask->getDisplayName() : std::string();
    //obj.sortIndex = fileTask->index();
    obj.uploadFileSize = task->getDataLength();
}

void MainWindow::showCodeForIndex(const QModelIndex& index) {
    std::vector<ZUploadObject> uploadObjects;
    auto item = uploadTreeModel_->getInternalItem(index);
    if (item->session) {
        int count = item->session->taskCount();
        for (int i = 0; i < count; i++) {
            auto task = item->session->getTask(i);
            if (task) {
                ZUploadObject obj;
                uploadTaskToUploadObject(task.get(), obj);
                uploadObjects.push_back(obj);
            }
        }
    }
    else if (item->task) {
        ZUploadObject obj;
        uploadTaskToUploadObject(item->task.get(), obj);
        uploadObjects.push_back(obj);
    }
    ResultsWindow dlg(uploadObjects);
    dlg.exec();
}

void MainWindow::onCustomContextMenu(const QPoint& point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (index.isValid()) {
        UploadTreeModel::InternalItem* internalItem = uploadTreeModel_->getInternalItem(index);
        QMenu* contextMenu = new QMenu(ui->treeView);
        //ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
        QAction* viewCodeAction = new QAction("View HTML/BBCode", contextMenu);

        contextMenu->addAction(viewCodeAction);
        connect(viewCodeAction, &QAction::triggered, [index, this](bool checked)
        {
            showCodeForIndex(index);
        });
        contextMenu->setDefaultAction(viewCodeAction);

        if (internalItem->task) {
            auto uploadResult = internalItem->task->uploadResult();
            QString url = QString::fromUtf8(uploadResult->getDirectUrl().c_str());
            if (url.isEmpty()) {
                url = QString::fromUtf8(uploadResult->getDownloadUrl().c_str());
            }
            if (!url.isEmpty()) {
                QAction* viewInBrowser = new QAction("Open in browser", contextMenu);
                connect(viewInBrowser, &QAction::triggered, [url](bool checked)
                {
                    QDesktopServices::openUrl(url);
                });
                contextMenu->addAction(viewInBrowser);
            }
        }

        contextMenu->exec(ui->treeView->viewport()->mapToGlobal(point));
    }
}

void MainWindow::onShowLog() {
    logWindow_->show();
    logWindow_->activateWindow();
}
