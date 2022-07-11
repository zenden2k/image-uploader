#ifndef IU_FUNC_IMAGEGENERATOR_H
#define IU_FUNC_IMAGEGENERATOR_H

#include "atlheaders.h"
#include "Core/BackgroundTask.h"

class ImageGeneratorTask: public BackgroundTask {
public:
	struct FileItem
	{
		CString fileName, title;
		FileItem(CString f, CString t): fileName(f), title(t){
			
		}
	};
	explicit ImageGeneratorTask(HWND wnd, std::vector<FileItem> files, int maxWidth, int maxHeight, CString mediaFile);
	
	BackgroundTaskResult doJob() override;

	CString outFileName() const;
	
	DISALLOW_COPY_AND_ASSIGN(ImageGeneratorTask);
private:
	CString outFileName_;
	std::vector<FileItem> files_;
	int maxWidth_;
	int maxHeight_;
	HWND wnd_;
	CString mediaFile_;
};


#endif