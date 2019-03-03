#include "FrameGrabberDlg.h"
#include "ui_FrameGrabberDlg.h"
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

void FrameGrabberDlg::frameGrabbed(const std::string& timeStr, int64_t time, AbstractImage* image) {
	QMetaObject::invokeMethod(qApp, [&] {
		//image.save("D:\\frame2.png");
		QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
		item->setText(U2Q(timeStr));
		QImage img = static_cast<QtImage*>(image)->toQImage();
		if (!img.isNull()) {
			QIcon ico(QPixmap::fromImage(img.scaledToWidth(150, Qt::SmoothTransformation)));
			item->setIcon(ico);
		}
		ui->listWidget->addItem(item);
	}, Qt::BlockingQueuedConnection);
	//ui->listWidget->settext(Qt::AlignCenter);
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
