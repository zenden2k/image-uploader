#include "ServiceLocator.h"

#include "Core/LocalFileCache.h"
#include "Core/HistoryManager.h"

class UploadManager;

class ServiceLocatorPrivate{
public:
    ServiceLocatorPrivate() {
        logger_ = nullptr;
        translator_ = nullptr;
#ifndef IU_SHELLEXT
        uploadErrorHandler_ = nullptr;
        engineList_ = nullptr;
        programWindow_ = nullptr;
        dialogProvider_ = nullptr;
        uploadManager_ = nullptr;
        myEngineList_ = nullptr;
        dispatcher_ = nullptr;
#endif
    }
    ILogger* logger_;
    ITranslator* translator_;
#ifndef IU_SHELLEXT
    CUploadEngineListBase* engineList_;
    CHistoryManager historyManager;
    IProgramWindow* programWindow_;
    IUploadErrorHandler* uploadErrorHandler_;
    IDialogProvider* dialogProvider_;
    ITaskDispatcher* dispatcher_;
    UploadManager* uploadManager_;
    CMyEngineList* myEngineList_;
    CLogWindow* logWindow_;
    BasicSettings* settings_;
#endif
};


ServiceLocator::ServiceLocator() : d_ptr(new ServiceLocatorPrivate()){
}


ILogger* ServiceLocator::logger() {
    return  d_ptr->logger_;
}

void ServiceLocator::setLogger(ILogger* logger) {
    d_ptr->logger_ = logger;
}

ITranslator* ServiceLocator::translator() {
    return d_ptr->translator_;
}

void ServiceLocator::setTranslator(ITranslator* transl) {
    d_ptr->translator_ = transl;
}

#ifndef IU_SHELLEXT
void ServiceLocator::setEngineList(CUploadEngineListBase* engineList) {
    d_ptr->engineList_ = engineList;
}

CUploadEngineListBase* ServiceLocator::engineList() {
    return d_ptr->engineList_;
}

LocalFileCache* ServiceLocator::localFileCache() {
    return LocalFileCache::instance();
}

CHistoryManager* ServiceLocator::historyManager() {
    return &d_ptr->historyManager;
}

IProgramWindow* ServiceLocator::programWindow() {
    return d_ptr->programWindow_;
}

IUploadErrorHandler* ServiceLocator::uploadErrorHandler() {
    return d_ptr->uploadErrorHandler_;
}

void ServiceLocator::setUploadErrorHandler(IUploadErrorHandler* errorHandler) {
    d_ptr->uploadErrorHandler_ = errorHandler;
}



IDialogProvider* ServiceLocator::dialogProvider() {
    return d_ptr->dialogProvider_;
}

void ServiceLocator::setDialogProvider(IDialogProvider* dialogProvider) {
    d_ptr->dialogProvider_ = dialogProvider;
}

ITaskDispatcher* ServiceLocator::taskDispatcher() {
    return d_ptr->dispatcher_;
}

void ServiceLocator::setTaskDispatcher(ITaskDispatcher* dispatcher) {
    d_ptr->dispatcher_ = dispatcher;
}

void ServiceLocator::setProgramWindow(IProgramWindow* window) {
    d_ptr->programWindow_ = window;
}


UploadManager* ServiceLocator::uploadManager() {
    return d_ptr->uploadManager_;
}

void ServiceLocator::setUploadManager(UploadManager* manager) {
    d_ptr->uploadManager_ = manager;
}

void ServiceLocator::setMyEngineList(CMyEngineList* list) {
    d_ptr->myEngineList_ = list;
}

CMyEngineList* ServiceLocator::myEngineList() const {
    return d_ptr->myEngineList_;
}

CLogWindow* ServiceLocator::logWindow() const {
    return d_ptr->logWindow_;
}

void ServiceLocator::setLogWindow(CLogWindow* logWindow) {
    d_ptr->logWindow_ = logWindow;
}

BasicSettings* ServiceLocator::basicSettings() const {
    return d_ptr->settings_;
}

void ServiceLocator::setSettings(BasicSettings* settingsInstance) {
    d_ptr->settings_ = settingsInstance;
}
#endif