#include "FrameGrabberDlg.h"

#include <QFileDialog>
#include "ui_FrameGrabberDlg.h"
#include "Core/Video/VideoGrabber.h"
#include "Core/CommonDefs.h"
#include "Core/Video/QtImage.h"

FrameGrabberDlg::FrameGrabberDlg(QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameGrabberDlg)
{
    ui->setupUi(this);
	ui->numOfFramesSpinBox->setValue(10);
	ui->stopButton->setVisible(false);
	ui->lineEdit->setText(fileName);
	connect(ui->stopButton, &QPushButton::clicked, this, &FrameGrabberDlg::onStopButtonClicked);
}

FrameGrabberDlg::~FrameGrabberDlg()
{
    delete ui;
}

void FrameGrabberDlg::frameGrabbed(const std::string& timeStr, int64_t time, AbstractImage* image) {
	if (!image) {
		return;
	}	
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

void FrameGrabberDlg::on_grabButton_clicked()
{
	ui->grabButton->setEnabled(false);
	ui->buttonBox->setEnabled(false);
	ui->stopButton->setVisible(true);
	ui->browseButton->setEnabled(false);

	grabber_.reset(new VideoGrabber());
	grabber_->setVideoEngine((VideoGrabber::VideoEngine) ui->comboBox->currentIndex());
	grabber_->onFrameGrabbed.bind(this, &FrameGrabberDlg::frameGrabbed);
	grabber_->onFinished.bind(this, &FrameGrabberDlg::onGrabFinished);
   // connect( grabber, SIGNAL(frameGrabbed(QString,QImage)), this, SLOT(frameGrabbed(QString,QImage))/*, Qt::BlockingQueuedConnection*/);
    /*connect(grabber, &VideoGrabber::finished,  [&]() {
        qDebug() << "finished";
        ui->label->setText("grabbing finished");
    });*/
	int frameCount = ui->numOfFramesSpinBox->value();
	if (frameCount < 1) {
		frameCount = 10;
	}
	grabber_->setFrameCount(frameCount);
	grabber_->grab(Q2U(ui->lineEdit->text()));
}

void FrameGrabberDlg::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if ( fileName.length() ) {
        ui->lineEdit->setText( fileName );
    }

}

void FrameGrabberDlg::onGrabFinished() {
	QMetaObject::invokeMethod(this, [&] {
		ui->grabButton->setEnabled(true);
		ui->buttonBox->setEnabled(true);
		ui->stopButton->setVisible(false);
		ui->browseButton->setEnabled(true);
	});
}

void FrameGrabberDlg::onStopButtonClicked() {
	if (grabber_) {
		grabber_->abort();
	}
}