#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QToolButton>
#include <Gui/FrameGrabberDlg.h>
#include <Gui/RegionSelect.h>
#include "models/uploadtreemodel.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/ServiceLocator.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/AppParams.h"
#include "Core/Settings.h"
//#include <QtWebKitWidgets/QWebView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ServiceLocator::instance()->setEngineList(&engineList_);
    //serversChanged_ = false;
   // engineList_.LoadFromFile(AppParams::instance()->dataDirectory() + "servers.xml", Settings.ServersSettings);

    scriptsManager_ = new ScriptsManager();
    IUploadErrorHandler* uploadErrorHandler = ServiceLocator::instance()->uploadErrorHandler();
    uploadEngineManager_ = new UploadEngineManager(&engineList_, uploadErrorHandler);
    uploadManager_ = new UploadManager(uploadEngineManager_, scriptsManager_, uploadErrorHandler);
    uploadTreeModel_ = new UploadTreeModel(this, uploadManager_);

    ui->treeView->setModel(uploadTreeModel_);
    /* ui->listWidget->viewport()->setAcceptDrops(true);
    ui->listWidget->installEventFilter(this);*/
    /*connect( ui->listWidget->model(), SIGNAL(layoutChanged()),
             this,  SLOT(updateView()));*/

    QToolButton *userInputButton = new QToolButton(ui->mainToolBar);
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
    ui->mainToolBar->addWidget(userInputButton);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if ( event->type() == QEvent::ChildRemoved ) {
        // ui->listWidget->setFlow(QListWidget::LeftToRight);
    }
    return false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete uploadManager_;
    delete uploadEngineManager_;
    delete scriptsManager_;
}



void  MainWindow::updateView() {
    qDebug() <<
                "moved";
    //ui->listWidget->setFlow(QListWidget::LeftToRight);
}


void MainWindow::on_actionGrab_frames_triggered()
{
    FrameGrabberDlg *dlg = new FrameGrabberDlg(this);
    dlg->exec();
}

void MainWindow::on_actionScreenshot_triggered()
{
    RegionSelect *selector;
    CScreenCaptureEngine eng;
    eng.setDelay(450);
    hide();
    eng.captureScreen();

    CScreenshotRegion* r;
    selector = new RegionSelect(0, eng.capturedBitmap());
    int resilt = selector->exec();

    if (resilt == QDialog::Accepted)
    {
        r = selector->selectedRegion();
    }
    else return;
    delete selector;

    if(!r) r = (new CActiveWindowRegion());

    eng.setSource(*eng.capturedBitmap());
    if(!eng.captureRegion(r))
    {

        delete r;
        return ;
    }

    delete r;
    QPixmap* screen  = eng.capturedBitmap();//QPixmap::grabWindow(QApplication::desktop()->winId());
    QString fileName = "C:/scr.png";
    screen->save(fileName);
    //addFileToList(fileName);
    show();

}

void MainWindow::on_actionAdd_files_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if ( fileName.length() ) {
        std::shared_ptr<UploadSession> uploadSession(new UploadSession());
        std::shared_ptr<FileUploadTask> task(new FileUploadTask(Q2U(fileName), IuCoreUtils::ExtractFileName(Q2U(fileName))));
        ServerProfile serverProfile("directupload.net");
        task->setServerProfile(serverProfile);
        uploadSession->addTask(task);
        uploadManager_->addSession(uploadSession);
        //ui->lineEdit->setText( fileName );
    }
}
