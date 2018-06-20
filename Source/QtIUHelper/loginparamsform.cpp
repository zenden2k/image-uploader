#include "loginparamsform.h"
#include "ui_loginparamsform.h"
#include <QMessageBox>

int findField(QList<PostField>& fields, QString regexp)
{
	for(int i=0; i<fields.size(); i++)
	{
		if(fields[i].name.contains(QRegExp(regexp)))
			return i;
	}
	return -1;
}

int findFirstField(QList<PostField>& fields, QStringList regexps)
{
	for(int i=0; i<regexps.size(); i++)
	{
		int res = findField(fields, regexps[i]);
		if(res >-1)
			return res;
	}
	return -1;
}

LoginParamsForm::LoginParamsForm(QList<PostField>& fields, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginParamsForm)
{
	ui->setupUi(this);
	ui->treeWidget->setColumnCount(2);
	QStringList headers;
	headers << tr("Field") << tr("Value");
	ui->treeWidget->setHeaderLabels(headers);


	for(int i=0; i<fields.size(); i++)
	{
		QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget);
		newItem->setText(0, fields[i].name);
		ui->loginComboBox->addItem(fields[i].name);
		newItem->setText(1,fields[i].value);
		ui->passwordComboBox->addItem(fields[i].name);
	}
	int loginFieldIndex = findFirstField(fields, QStringList()<<"login"<<"user(\\w+)name"<<"user(\\w+)id"<<"user");
	int passwordFieldIndex = findFirstField(fields, QStringList()<<"password"<<"pwd"<<"pass"<<"pswd");
	if(loginFieldIndex >= 0)
		ui->loginComboBox->setCurrentIndex( loginFieldIndex);
	if(passwordFieldIndex >= 0)
		ui->passwordComboBox->setCurrentIndex( passwordFieldIndex);
}


LoginParamsForm::~LoginParamsForm()
{
    delete ui;
}

int LoginParamsForm::getLoginFieldIndex()
{
	return ui->loginComboBox->currentIndex();
}

int LoginParamsForm::getPasswordFieldIndex()
{
	return ui->passwordComboBox->currentIndex();
}

 void LoginParamsForm::accept()
 {
		if(ui->loginComboBox->currentIndex()<0 || ui->passwordComboBox->currentIndex()<0)
		{
			QMessageBox::warning(this, "Warning","You must fill both fields!");
		}
		else QDialog::accept();
 }
