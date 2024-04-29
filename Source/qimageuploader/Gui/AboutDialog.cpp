#include "AboutDialog.h"

#include "ui_AboutDialog.h"
#include "Core/AppParams.h"
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent): 
    QDialog(parent),
    ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    auto version = AppParams::instance()->GetAppVersion();

    ui->versionLabel->setText(QString::fromStdString(version->FullVersion) + " build " + QString::number(version->Build));
    QPushButton * btn = new QPushButton(tr("About Qt"), this);
    connect(btn, &QPushButton::clicked, this, [] {
        qApp->aboutQt();
    });

    ui->buttonBox->addButton(btn, QDialogButtonBox::ActionRole);
}

AboutDialog::~AboutDialog() {
    
}
