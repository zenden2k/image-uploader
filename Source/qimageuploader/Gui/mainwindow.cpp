#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <QDebug>
#include <QFileDialog>
#include <QToolButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <Gui/FrameGrabberDlg.h>
#include <Gui/RegionSelect.h>
#include "models/uploadtreemodel.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/ServiceLocator.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/AppParams.h"
#include "Core/Settings.h"
#include "Core/Network/NetworkClientFactory.h"
#include "ResultsWindow.h"
#include "Core/OutputCodeGenerator.h"

//#include <QtWebKitWidgets/QWebView>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ServiceLocator::instance()->setEngineList(&engineList_);

	if (!engineList_.loadFromFile(AppParams::instance()->dataDirectory() + "servers.xml", Settings.ServersSettings)){
		QMessageBox::warning(this, "Failure", "Unable to load servers.xml");
	}

    auto networkClientFactory = std::make_shared<NetworkClientFactory>();
    scriptsManager_ = new ScriptsManager(networkClientFactory);
    IUploadErrorHandler* uploadErrorHandler = ServiceLocator::instance()->uploadErrorHandler();
    uploadEngineManager_ = new UploadEngineManager(&engineList_, uploadErrorHandler, networkClientFactory);
    uploadManager_ = new UploadManager(uploadEngineManager_,&engineList_, scriptsManager_, uploadErrorHandler, networkClientFactory);
    uploadTreeModel_ = new UploadTreeModel(this, uploadManager_);

    ui->treeView->setModel(uploadTreeModel_);
	ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui->treeView, &QTreeView::doubleClicked, this, &MainWindow::itemDoubleClicked);

	connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::onCustomContextMenu);

    /* ui->listWidget->viewport()->setAcceptDrops(true);
    ui->listWidget->installEventFilter(this);*/
    /*connect( ui->listWidget->model(), SIGNAL(layoutChanged()),
             this,  SLOT(updateView()));*/

    /*QToolButton *userInputButton = new QToolButton(ui->mainToolBar);
    QMenu *userInputMenu = new QMenu(userInputButton);
    userInputButton->setIcon(QIcon(":/res/images.ico"));
    userInputButton->setIconSize(QSize(16,16));
    userInputButton->setText("Add Images");
    userInputButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //  userInputButton->setFixedHeight(20);
    userInputButton->setMenu(userInputMenu);
    userInputButton->setPopupMode(QToolButton::InstantPopup);

    // userInputButton->setPopupMode(QToolButton::MenuButtonPopup);
    userInputMenu->addAction(ui->actionAdd_files);
    userInputMenu->addAction(ui->actionAdd_Images);
    ui->mainToolBar->addWidget(userInputButton);*/
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if ( event->type() == QEvent::ChildRemoved ) {
        // ui->listWidget->setFlow(QListWidget::LeftToRight);
    }
    return false;
}

MainWindow::~MainWindow()
{
    delete uploadManager_;
    delete uploadEngineManager_;
    delete scriptsManager_;
}

void  MainWindow::updateView() {

}

void MainWindow::on_actionGrab_frames_triggered()
{
    FrameGrabberDlg dlg;
    dlg.exec();
}

void MainWindow::on_actionScreenshot_triggered()
{
    std::unique_ptr<RegionSelect> selector;
    CScreenCaptureEngine eng;
    eng.setDelay(450);
    hide();
    eng.captureScreen();

    std::unique_ptr<CScreenshotRegion> r;
    selector.reset(new RegionSelect(0, eng.capturedBitmap()));
    int resilt = selector->exec();

    if (resilt == QDialog::Accepted)
    {
        r.reset(selector->selectedRegion());
    }
    else return;

    if(!r) r.reset(new CActiveWindowRegion());

    eng.setSource(*eng.capturedBitmap());
    if(!eng.captureRegion(r.get()))
    {
        return ;
    }

    QPixmap* screen  = eng.capturedBitmap();//QPixmap::grabWindow(QApplication::desktop()->winId());
    QString fileName = "d:/scr.png";
	if (screen->save(fileName)) {
		addFileToList(fileName);
	}
    show();

}

void MainWindow::on_actionAdd_files_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this);
    if (!fileNames.isEmpty()) {
        auto uploadSession = std::make_shared<UploadSession>();
		for (const auto& fileName : fileNames) {
			auto task = std::make_shared<FileUploadTask>(Q2U(fileName), IuCoreUtils::ExtractFileName(Q2U(fileName)));
			ServerProfile serverProfile("directupload.net");
			task->setServerProfile(serverProfile);
			uploadSession->addTask(task);
		}

        uploadManager_->addSession(uploadSession);
    }
}

bool MainWindow::addFileToList(QString fileName) {
	auto uploadSession = std::make_shared<UploadSession>();
	auto task = std::make_shared<FileUploadTask>(Q2U(fileName), IuCoreUtils::ExtractFileName(Q2U(fileName)));
	ServerProfile serverProfile("directupload.net");
	task->setServerProfile(serverProfile);
	uploadSession->addTask(task);
	uploadManager_->addSession(uploadSession);
	ui->treeView->expandAll(); 
	return true;
}

void MainWindow::itemDoubleClicked(const QModelIndex &index) {
	showCodeForIndex(index);
}

void MainWindow::uploadTaskToUploadObject(UploadTask* task, ZUploadObject& obj) {
	
	/*HistoryItem hi;
	hi.localFilePath = fileTask->originalFileName();
	hi.serverName = fileTask->serverProfile().serverName();*/
	obj.serverName = task->serverProfile().serverName();
	FileUploadTask* fileTask = dynamic_cast<FileUploadTask*> (task);

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

void MainWindow::onCustomContextMenu(const QPoint &point)
{
	QModelIndex index = ui->treeView->indexAt(point);
	if (index.isValid()) {
		UploadTreeModel::InternalItem* internalItem = uploadTreeModel_->getInternalItem(index);
		QMenu* contextMenu = new QMenu(ui->treeView);
		//ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
		QAction* viewCodeAction = new QAction("View HTML/BBCode", contextMenu);
		contextMenu->addAction(viewCodeAction);
		connect(viewCodeAction, &QAction::triggered, [index, this](bool checked) {
			showCodeForIndex(index);
		});
		if (internalItem->task) { 
			auto uploadResult = internalItem->task->uploadResult();
			QString url = QString::fromUtf8(uploadResult->getDirectUrl().c_str());
			if (url.isEmpty()) {
				url = QString::fromUtf8(uploadResult->getDownloadUrl().c_str());
			}
			if (!url.isEmpty()) {
				QAction* viewInBrowser = new QAction("Open in browser", contextMenu);
				connect(viewInBrowser, &QAction::triggered, [url](bool checked) {
					QDesktopServices::openUrl(url);
				});
				contextMenu->addAction(viewInBrowser);
			}
		}

		contextMenu->exec(ui->treeView->viewport()->mapToGlobal(point));
	}
}