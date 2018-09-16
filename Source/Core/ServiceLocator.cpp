#include "ServiceLocator.h"

#include "Core/LocalFileCache.h"
#include "Core/HistoryManager.h"

class UploadManager;

class ServiceLocatorPrivate{
public:
    ServiceLocatorPrivate() {
        engineList_ = nullptr;
        programWindow_ = nullptr;
        uploadErrorHandler_ = nullptr;
        logger_ = nullptr;
        dialogProvider_ = nullptr;
        translator_ = nullptr;
        uploadManager_ = nullptr;
    }
    CUploadEngineList_Base* engineList_;
    CHistoryManager historyManager;
    IProgramWindow* programWindow_;
    IUploadErrorHandler* uploadErrorHandler_;
    ILogger* logger_;
    IDialogProvider* dialogProvider_;
    ITranslator* translator_;
    ITaskDispatcher* dispatcher_;
    UploadManager* uploadManager_;
};

ServiceLocator::ServiceLocator() : d_ptr(new ServiceLocatorPrivate()){
}

void ServiceLocator::setEngineList(CUploadEngineList_Base* engineList) {
    d_ptr->engineList_ = engineList;
}

CUploadEngineList_Base* ServiceLocator::engineList() {
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

ILogger* ServiceLocator::logger() {
    return  d_ptr->logger_;
}

void ServiceLocator::setLogger(ILogger* logger) {
    d_ptr->logger_ = logger;
}

IDialogProvider* ServiceLocator::dialogProvider() {
    return d_ptr->dialogProvider_;
}

void ServiceLocator::setDialogProvider(IDialogProvider* dialogProvider) {
    d_ptr->dialogProvider_ = dialogProvider;
}

ITranslator* ServiceLocator::translator() {
    return d_ptr->translator_;
}

void ServiceLocator::setTranslator(ITranslator* transl) {
    d_ptr->translator_ = transl;
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
