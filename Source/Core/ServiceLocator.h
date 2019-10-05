#ifndef IU_CORE_SERVICELOCATOR_H
#define IU_CORE_SERVICELOCATOR_H

#include "Utils/Singleton.h"

#include <memory>

#include "ProgramWindow.h"
#include "Core/Logging/Logger.h"
#include "Network/INetworkClient.h"

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
class UrlShorteningFilter;

class ServiceLocator : public Singleton<ServiceLocator> {
public:
    ServiceLocator();
    std::shared_ptr<ILogger> logger();
    void setLogger(std::shared_ptr<ILogger> logger);
    ITranslator* translator();
    void setTranslator(ITranslator* transl);
    IDialogProvider* dialogProvider();
    void setDialogProvider(IDialogProvider* dialogProvider);
    std::shared_ptr<IUploadErrorHandler> uploadErrorHandler();
    void setUploadErrorHandler(std::shared_ptr<IUploadErrorHandler> errorHandler);
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
    std::shared_ptr<UrlShorteningFilter> urlShorteningFilter() const;
    void setUrlShorteningFilter(std::shared_ptr<UrlShorteningFilter> filter);

    template<class T> T* settings() const {
        T* res = dynamic_cast<T*>(basicSettings());
        //assert(res);
        return res;
    }

    void setNetworkClientFactory(std::shared_ptr<INetworkClientFactory> factory);
    std::shared_ptr<INetworkClientFactory> networkClientFactory() const;

protected:
    std::shared_ptr<ServiceLocatorPrivate> d_ptr; // unique_ptr won't compile with incomplete type
};
#endif
