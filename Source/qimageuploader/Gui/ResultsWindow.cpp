#include "ResultsWindow.h"

#include <QClipboard>

#include "ui_ResultsWindow.h"

ResultsWindow::ResultsWindow(std::vector<ZUploadObject> objects, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultsWindow),
	uploadObjects_(objects)
{
    ui->setupUi(this);
	ui->tabBar->addTab("For forums (BBCode)");
	ui->tabBar->addTab("For websites (HTML)");
	ui->tabBar->addTab("Markdown");
	ui->tabBar->addTab("Just links");    
	ui->codeTypeCombo->addItems(QStringList() << "Table of clickable thumbnails" << "Clickable thumbnails"
		<< "Images" << "Links");
	connect(ui->tabBar, &QTabBar::currentChanged, this, &ResultsWindow::currentTabChanged);
	connect(ui->codeTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(generateCode()));
	connect(ui->copyToClipboardButton, &QPushButton::clicked, this, &ResultsWindow::onCopyToClipboard);
	generateCode();
}


ResultsWindow::~ResultsWindow()
{
}

void ResultsWindow::generateCode() {
	ZOutputCodeGenerator generator;
	ZOutputCodeGenerator::CodeLang lang;
	ZOutputCodeGenerator::CodeType codeType;
	int tabIndex = ui->tabBar->currentIndex();
	if (tabIndex == 0) {
		lang = ZOutputCodeGenerator::clBBCode;
	}
	else if (tabIndex == 1) {
		lang = ZOutputCodeGenerator::clHTML;
	}
	else if (tabIndex == 2) {
		lang = ZOutputCodeGenerator::clHTML;
	}
	else {
		lang = ZOutputCodeGenerator::clPlain;
	}
	int codeTypeIndex = ui->codeTypeCombo->currentIndex();
	if (codeTypeIndex == 0) {
		codeType = ZOutputCodeGenerator::ctTableOfThumbnails;
	}
	else if (codeTypeIndex == 1) {
		codeType = ZOutputCodeGenerator::ctClickableThumbnails;
	}
	else if (codeTypeIndex == 2) {
		codeType = ZOutputCodeGenerator::ctImages;
	}
	else if (codeTypeIndex == 3) {
		codeType = ZOutputCodeGenerator::ctLinks;
	}
	generator.setLang(lang);
	generator.setType(codeType);
	std::string res = generator.generate(uploadObjects_);
	ui->plainTextEdit->setPlainText(QString::fromUtf8(res.c_str()));
}

void ResultsWindow::currentTabChanged(int index) {
	ui->codeTypeCombo->setEnabled(index != 3); // Disable combo on "Just links" tab
	generateCode();
}

void ResultsWindow::onCopyToClipboard() {
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(ui->plainTextEdit->toPlainText());
}