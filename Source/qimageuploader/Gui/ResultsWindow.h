#ifndef QIMAGEUPLOADER_GUI_RESULTSWINDOW_H
#define QIMAGEUPLOADER_GUI_RESULTSWINDOW_H

#include <QDialog>
#include <vector>
#include <memory>
#include "Core/OutputCodeGenerator.h"

namespace Ui {
class ResultsWindow;
}

class ResultsWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit ResultsWindow(std::vector<UploadObject> uploadObjects, QWidget *parent = 0);
    ~ResultsWindow();
protected:
	
private slots:
	void generateCode();
	void currentTabChanged(int index);
	void onCopyToClipboard();
private:
    std::unique_ptr<Ui::ResultsWindow> ui;
	std::vector<UploadObject> uploadObjects_;
};

#endif // FRAMEGRABBERDLG_H
