#include "AboutDialog.h"

#include "ui_AboutDialog.h"
#include "Core/AppParams.h"

AboutDialog::AboutDialog(QWidget *parent): 
    QDialog(parent),
    ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    auto version = AppParams::instance()->GetAppVersion();

    ui->versionLabel->setText(QString::fromStdString(version->FullVersion) + " build " + QString::number(version->Build));
}

AboutDialog::~AboutDialog() {
    
}
