#ifndef LOGINPARAMSFORM_H
#define LOGINPARAMSFORM_H

#include <QDialog>
#include "qhelpernam.h"
namespace Ui {
    class LoginParamsForm;
}

class LoginParamsForm : public QDialog
{
    Q_OBJECT

public:
	 explicit LoginParamsForm(QList<PostField>& fields, QWidget *parent=0);
    ~LoginParamsForm();
	int getLoginFieldIndex();
	int getPasswordFieldIndex();
private:
    Ui::LoginParamsForm *ui;
	 void accept();

private slots:

};

#endif // LOGINPARAMSFORM_H
