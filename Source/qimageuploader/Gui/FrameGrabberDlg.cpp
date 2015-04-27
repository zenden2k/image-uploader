#include "FrameGrabberDlg.h"
#include "ui_FrameGrabberDlg.h"
#include "Core/Video/DirectshowFrameGrabber.h"
#include "Core/Video/VideoGrabber.h"
#include <QFileDialog>
#include "Core/CommonDefs.h"
#include "Core/Video/QtImage.h"

FrameGrabberDlg::FrameGrabberDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameGrabberDlg)
{
    ui->setupUi(this);
}

FrameGrabberDlg::~FrameGrabberDlg()
{
    delete ui;
}

void FrameGrabberDlg::frameGrabbed(const Utf8String& timeStr, int64_t time, AbstractImage* image) {
    //image.save("D:\\frame2.png");
    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
    item->setText(U2Q(timeStr));
    QImage img = static_cast<QtImage*>(image)->toQImage();
    item->setIcon( QIcon( QPixmap::fromImage(img.scaledToWidth(128, Qt::SmoothTransformation)) ));
    ui->listWidget->addItem(item);
   // ui->label->setPixmap(QPixmap::fromImage(image) );
}

void FrameGrabberDlg::on_pushButton_clicked()
{
    VideoGrabber* grabber = new VideoGrabber();
    grabber->setVideoEngine((VideoGrabber::VideoEngine) ui->comboBox->currentIndex());
    grabber->onFrameGrabbed.bind(this, &FrameGrabberDlg::frameGrabbed);
   // connect( grabber, SIGNAL(frameGrabbed(QString,QImage)), this, SLOT(frameGrabbed(QString,QImage))/*, Qt::BlockingQueuedConnection*/);
    /*connect(grabber, &VideoGrabber::finished,  [&]() {
        qDebug() << "finished";
        ui->label->setText("grabbing finished");
    });*/

    grabber->setFrameCount(15);
    grabber->grab(Q2U(ui->lineEdit->text()));
}

void FrameGrabberDlg::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if ( fileName.length() ) {
        ui->lineEdit->setText( fileName );
    }

}
