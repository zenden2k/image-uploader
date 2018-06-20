#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qhelpernam.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        QHelperNAM * m_nam;
         QString generatedConfigTemplate;
            QString loginStr;
    private slots:
             void on_analisePage_clicked();
      void on_goButton_clicked();
             void namUrlLoad(QString url);
             void updateCode();
             void onUrlChanged(QUrl url);
             void onPost(QString postUrl, QList<PostField>* fields);
             void onUpload(QString uploadUrl, QList<PostField>* fields);
};

#endif // MAINWINDOW_H
