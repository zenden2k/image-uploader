#include "ResultsWindow.h"

#include <QClipboard>
#include "Core/OutputGenerator/OutputGeneratorFactory.h"

#include "ui_ResultsWindow.h"
namespace OutputCodeGenerator = ImageUploader::Core::OutputGenerator;
ResultsWindow::ResultsWindow(std::vector<OutputCodeGenerator::UploadObject> objects, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ResultsWindow),
    uploadObjects_(objects) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
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


ResultsWindow::~ResultsWindow() {
}

void ResultsWindow::generateCode() {
    OutputCodeGenerator::OutputGeneratorFactory factory;

    OutputCodeGenerator::CodeLang lang;
    OutputCodeGenerator::CodeType codeType;
    OutputCodeGenerator::GeneratorID gid;
    int tabIndex = ui->tabBar->currentIndex();
    if (tabIndex == 0) {
        //lang = OutputCodeGenerator::clBBCode;
        gid = OutputCodeGenerator::gidBBCode;
    }
    else if (tabIndex == 1) {
        gid = OutputCodeGenerator::gidHTML;
    }
    else if (tabIndex == 2) {
        gid = OutputCodeGenerator::gidMarkdown;
    }
    else {
        gid = OutputCodeGenerator::gidPlain;
    }
    int codeTypeIndex = ui->codeTypeCombo->currentIndex();
    if (codeTypeIndex == 0) {
        codeType = OutputCodeGenerator::ctTableOfThumbnails;
    }
    else if (codeTypeIndex == 1) {
        codeType = OutputCodeGenerator::ctClickableThumbnails;
    }
    else if (codeTypeIndex == 2) {
        codeType = OutputCodeGenerator::ctImages;
    }
    else if (codeTypeIndex == 3) {
        codeType = OutputCodeGenerator::ctLinks;
    } else {
        codeType = OutputCodeGenerator::ctTableOfThumbnails;
    }
    auto generator = factory.createOutputGenerator(gid, codeType);
    generator->setType(codeType);
    std::string res = generator->generate(uploadObjects_);
    ui->plainTextEdit->setPlainText(QString::fromStdString(res));
}

void ResultsWindow::currentTabChanged(int index) {
    ui->codeTypeCombo->setEnabled(index != 3); // Disable combo on "Just links" tab
    generateCode();
}

void ResultsWindow::onCopyToClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(ui->plainTextEdit->toPlainText());
}
