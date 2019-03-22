#include "FrameGrabberDlg.h"

#include <QFileDialog>
#include <QTemporaryFile>
#include "ui_FrameGrabberDlg.h"
#include "Core/Video/VideoGrabber.h"
#include "Core/CommonDefs.h"
#include "Core/Video/QtImage.h"
#include "Core/AppParams.h"

Q_DECLARE_METATYPE(AbstractImage*)


FrameGrabberDlg::FrameGrabberDlg(QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameGrabberDlg)
{
    qRegisterMetaType<AbstractImage*>("AbstractImage*");
    ui->setupUi(this);
	ui->numOfFramesSpinBox->setValue(10);
	ui->stopButton->setVisible(false);
	ui->lineEdit->setText(fileName);
    ui->comboBox->addItem("Auto", QVariant(int(VideoGrabber::veAuto)));
    ui->comboBox->addItem("Avcodec", QVariant(int(VideoGrabber::veAvcodec)));
#ifdef _WIN32
    ui->comboBox->addItem("Directshow", QVariant(int(VideoGrabber::veDirectShow)));
#endif
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
    QString timeString = U2Q(timeStr);
    QImage img = static_cast<QtImage*>(image)->toQImage();

    if (!img.isNull()) {
       
        QTemporaryFile f(U2Q(AppParams::instance()->tempDirectory()) + "/grab_XXXXXX.png");
        f.setAutoRemove(false);
        QString uniqueFileName;
        if (f.open()) {
            uniqueFileName = f.fileName();
            f.close();
        }
        if (!uniqueFileName.isEmpty()) {
            if (img.save(uniqueFileName)) {
                
                QIcon ico(QPixmap::fromImage(img.scaledToWidth(150, Qt::SmoothTransformation)));
                
                QMetaObject::invokeMethod(this, "frameGrabbedSlot", Qt::BlockingQueuedConnection,
                    Q_ARG(QString, timeString), Q_ARG(QString, uniqueFileName), Q_ARG(QIcon, ico));
            }
        }
    }

    
		
	//ui->listWidget->settext(Qt::AlignCenter);
   // ui->label->setPixmap(QPixmap::fromImage(image) );
}

void FrameGrabberDlg::frameGrabbedSlot(QString timeStr, QString fileName, QIcon ico) {
    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
    item->setText(timeStr);
    item->setData(Qt::UserRole, fileName);
    item->setIcon(ico);
    ui->listWidget->addItem(item);
}

void FrameGrabberDlg::on_grabButton_clicked()
{
	ui->grabButton->setEnabled(false);
	ui->buttonBox->setEnabled(false);
	ui->stopButton->setVisible(true);
	ui->browseButton->setEnabled(false);

	grabber_.reset(new VideoGrabber());
	grabber_->setVideoEngine((VideoGrabber::VideoEngine) ui->comboBox->currentData().toInt());
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
	QMetaObject::invokeMethod(this, "grabFinishedSlot", Qt::BlockingQueuedConnection);
}

void FrameGrabberDlg::grabFinishedSlot() {
    ui->grabButton->setEnabled(true);
    ui->buttonBox->setEnabled(true);
    ui->stopButton->setVisible(false);
    ui->browseButton->setEnabled(true);
}

void FrameGrabberDlg::onStopButtonClicked() {
	if (grabber_) {
		grabber_->abort();
	}
}

void FrameGrabberDlg::getGrabbedFrames(QStringList& fileNames) const {
    int itemCount = ui->listWidget->count();
    for ( int i = 0; i< itemCount; i++) {
        auto item = ui->listWidget->item(i);
        fileNames.push_back(item->data(Qt::UserRole).toString());
    }
}