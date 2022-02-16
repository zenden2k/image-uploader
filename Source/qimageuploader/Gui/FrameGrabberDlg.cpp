#include "FrameGrabberDlg.h"

#include <QFileDialog>
#include <QTemporaryFile>
#include "ui_FrameGrabberDlg.h"
#include "Core/Video/VideoGrabber.h"
#include "Core/CommonDefs.h"
#include "Core/Video/QtImage.h"
#include "Core/AppParams.h"
#include <QDesktopServices>
#include <QDebug>

Q_DECLARE_METATYPE(AbstractImage*)

FrameGrabberDlg::FrameGrabberDlg(QString fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameGrabberDlg)
{
    qRegisterMetaType<AbstractImage*>("AbstractImage*");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
	ui->numOfFramesSpinBox->setValue(10);
	ui->stopButton->setVisible(false);
	ui->lineEdit->setText(fileName);
    ui->comboBox->addItem("Auto", QVariant(int(VideoGrabber::veAuto)));
#ifdef IU_ENABLE_FFMPEG
    ui->comboBox->addItem("Avcodec", QVariant(int(VideoGrabber::veAvcodec)));
#endif

#ifdef _WIN32
    ui->comboBox->addItem("Directshow", QVariant(int(VideoGrabber::veDirectShow)));
    ui->comboBox->addItem("Directshow2", QVariant(int(VideoGrabber::veDirectShow2)));
#endif
    ui->progressRing->hide();
	connect(ui->stopButton, &QPushButton::clicked, this, &FrameGrabberDlg::onStopButtonClicked);
    connect(ui->listWidget, &QListWidget::doubleClicked, this, &FrameGrabberDlg::itemDoubleClicked);

}

FrameGrabberDlg::~FrameGrabberDlg()
{
    delete ui;
}

void FrameGrabberDlg::frameGrabbed(const std::string& timeStr, int64_t time, std::shared_ptr<AbstractImage> image) {
	if (!image) {
		return;
	}
    QString timeString = U2Q(timeStr);
	auto qtImage = dynamic_cast<QtImage*>(image.get());
	if (!qtImage) {
	    return;
	}
    QImage img = qtImage->toQImage();

    if (!img.isNull()) {
        QString tempDirectory = U2Q(AppParams::instance()->tempDirectory());
        QTemporaryFile f(tempDirectory + "/grab_XXXXXX.png");
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

	grabber_ = std::make_unique<VideoGrabber>();
	grabber_->setVideoEngine(static_cast<VideoGrabber::VideoEngine>(ui->comboBox->currentData().toInt()));
    using namespace std::placeholders;
	grabber_->setOnFrameGrabbed(std::bind(&FrameGrabberDlg::frameGrabbed, this, _1, _2, _3));
	grabber_->setOnFinished(std::bind(&FrameGrabberDlg::onGrabFinished, this));

	int frameCount = ui->numOfFramesSpinBox->value();
	if (frameCount < 1) {
		frameCount = 10;
	}
	grabber_->setFrameCount(frameCount);
    ui->progressRing->show();
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
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    ui->progressRing->hide();
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


void FrameGrabberDlg::itemDoubleClicked(const QModelIndex& index) {
    QListWidgetItem* item = ui->listWidget->item(index.row());
    if (item) {
        QString fileName = item->data(Qt::UserRole).toString();
        QDesktopServices::openUrl("file:///" + fileName);
    }
}