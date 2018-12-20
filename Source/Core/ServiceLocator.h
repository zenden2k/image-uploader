#ifndef IU_CORE_SERVICELOCATOR_H
#define IU_CORE_SERVICELOCATOR_H

#include "Utils/Singleton.h"

#include <memory>
#include "ProgramWindow.h"
#include "Core/Logging/Logger.h"

class UploadManager;
class IDialogProvider;
class IUploadErrorHandler;
class CUploadEngineListBase;
class LocalFileCache;
class CHistoryManager;
class ServiceLocatorPrivate;
class ITranslator;
class ITaskDispatcher;
class CMyEngineList;

class ServiceLocator : public Singleton<ServiceLocator> {
public:
    ServiceLocator();
    void setEngineList(CUploadEngineListBase* engineList);
    CUploadEngineListBase* engineList();
    LocalFileCache* localFileCache();
    CHistoryManager* historyManager();
    IProgramWindow* programWindow();
    void setProgramWindow(IProgramWindow* window);
    IUploadErrorHandler* uploadErrorHandler();
    void setUploadErrorHandler(IUploadErrorHandler* errorHandler);
    ILogger* logger();
    void setLogger(ILogger* logger);
    IDialogProvider* dialogProvider();
    void setDialogProvider(IDialogProvider* dialogProvider);
    ITranslator* translator();
    void setTranslator(ITranslator* transl);
    ITaskDispatcher* taskDispatcher();
    void setTaskDispatcher(ITaskDispatcher* dispatcher);
    UploadManager* uploadManager();
    void setUploadManager(UploadManager* manager);
    void setMyEngineList(CMyEngineList* list);
    CMyEngineList* myEngineList() const;

protected:
    std::shared_ptr<ServiceLocatorPrivate> d_ptr; // unique_ptr won't compile with incomplete type
};
#endif
