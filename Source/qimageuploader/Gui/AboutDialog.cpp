#include "AboutDialog.h"

#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent): 
    QDialog(parent),
    ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->versionLabel->setText("v0.1.1");
}

AboutDialog::~AboutDialog() {
    
}
