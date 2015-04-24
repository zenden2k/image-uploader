#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QDebug>
#include <QFileDialog>
#include <QToolButton>
#include <Gui/FrameGrabberDlg.h>
#include <Gui/RegionSelect.h>
//#include <QtWebKitWidgets/QWebView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
/*
           QString style =  "QToolButton {\n"
                           "   border: none;\n"




                           "}\n"
                           "QToolButton:hover {\n"
                   " background-color:#c2e0ff;"
                   "border: 1px solid #3399ff;"


                           "}\n"
                           "QToolButton:pressed {\n"
                   " border: 1px solid #828790;"
                   "border-bottom:0;"
                           "  \n}\n"
                   " QToolButton::menu-indicator  { "

                   " subcontrol-position: right center;"
                   " subcontrol-origin: content;"

                   "image: url(:/res/dropdown.ico);"
                   "color: red;"
                   "}"
                   " QToolButton::menu-indicator:hover  { "

                   " subcontrol-position: right center;"
                   " subcontrol-origin: border;"
                   "}"
                   " QToolButton::menu-indicator:pressed  { "

                   " subcontrol-position: right center;"
                   " subcontrol-origin: border;"
                   "}"

                   ;
            userInputButton->setStyleSheet(style);
            userInputMenu->setStyleSheet(
                        "QMenu {"

                        " border: 1px solid #828790;"
"background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333);}"

                                           "}"
                        "QMenu::item{"
                        "margin: 1px;"
                        "padding: 3px;"
                        "}"

                        "QMenu::item:selected{"
                        "border: 1px solid  #3399ff;"
                        "background-color: #c4e1ff"
                        "}"
                        "QMenu::indicator {"
                       " width: 13px;"
                        " height: 13px;"
                        "  }"

                                           );*/

           // userInputButton->setPopupMode(QToolButton::MenuButtonPopup);
            userInputMenu->addAction(ui->actionAdd_files);
         userInputMenu->addAction(ui->actionAdd_Images);
ui->mainToolBar->addWidget(userInputButton);
   // ui->centralWidget->layout()->addWidget(view);
    //view->setUrl( QUrl("http://zenden.ws"));

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
}



void  MainWindow::updateView() {
    qDebug() <<
                "moved";
    //ui->listWidget->setFlow(QListWidget::LeftToRight);
}

void MainWindow::on_pushButton_3_clicked()
{
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
