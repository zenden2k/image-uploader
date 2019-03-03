#ifndef QIMAGEUPLOADER_GUI_RESULTSWINDOW_H
#define QIMAGEUPLOADER_GUI_RESULTSWINDOW_H

#include <QDialog>
#include <vector>

#include "Core/OutputCodeGenerator.h"

namespace Ui {
class ResultsWindow;
}

class ResultsWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit ResultsWindow(std::vector<ZUploadObject> uploadObjects, QWidget *parent = 0);
    ~ResultsWindow();
protected:
	
private slots:
	void generateCode();
	void currentTabChanged(int index);
private:
    std::unique_ptr<Ui::ResultsWindow> ui;
	std::vector<ZUploadObject> uploadObjects_;
};

#endif // FRAMEGRABBERDLG_H
