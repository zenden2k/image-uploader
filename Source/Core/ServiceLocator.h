#ifndef IU_CORE_SERVICELOCATOR_H
#define IU_CORE_SERVICELOCATOR_H

#include "Utils/Singleton.h"

#include <memory>
#include <cassert>

#include "ProgramWindow.h"
#include "Core/Logging/Logger.h"

class BasicSettings;
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
class CLogWindow;

class ServiceLocator : public Singleton<ServiceLocator> {
public:
    ServiceLocator();
    ILogger* logger();
    void setLogger(ILogger* logger);
    ITranslator* translator();
    void setTranslator(ITranslator* transl);
#ifndef IU_SHELLEXT
    IDialogProvider* dialogProvider();
    void setDialogProvider(IDialogProvider* dialogProvider);
    IUploadErrorHandler* uploadErrorHandler();
    void setUploadErrorHandler(IUploadErrorHandler* errorHandler);
    ITaskDispatcher* taskDispatcher();
    void setTaskDispatcher(ITaskDispatcher* dispatcher);
    UploadManager* uploadManager();
    void setUploadManager(UploadManager* manager);
    void setMyEngineList(CMyEngineList* list);
    CMyEngineList* myEngineList() const;
    void setEngineList(CUploadEngineListBase* engineList);
    CUploadEngineListBase* engineList();
    LocalFileCache* localFileCache();
    CHistoryManager* historyManager();
    IProgramWindow* programWindow();
    void setProgramWindow(IProgramWindow* window);
    CLogWindow* logWindow() const;
    void setLogWindow(CLogWindow* logWindow);
    BasicSettings* basicSettings() const;
    void setSettings(BasicSettings* settingsInstance);

    template<class T> T* settings() const {
        T* res = dynamic_cast<T*>(basicSettings());
        //assert(res);
        return res;
    }
#endif

protected:
    std::shared_ptr<ServiceLocatorPrivate> d_ptr; // unique_ptr won't compile with incomplete type
};
#endif
