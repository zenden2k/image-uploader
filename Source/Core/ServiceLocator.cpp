#include "ServiceLocator.h"

#include "Core/LocalFileCache.h"
#include "Core/HistoryManager.h"

class UploadManager;

class ServiceLocatorPrivate{
public:
    ServiceLocatorPrivate() {
        translator_ = nullptr;
        engineList_ = nullptr;
        programWindow_ = nullptr;
        dialogProvider_ = nullptr;
        uploadManager_ = nullptr;
        myEngineList_ = nullptr;
        taskRunner_ = nullptr;
        logWindow_ = nullptr;
        settings_ = nullptr;
        taskDispatcher_ = nullptr;
    }
    std::shared_ptr<ILogger> logger_;
    ITranslator* translator_;
    CUploadEngineListBase* engineList_;
    CHistoryManager historyManager;
    IProgramWindow* programWindow_;
    std::shared_ptr<IUploadErrorHandler> uploadErrorHandler_;
    IDialogProvider* dialogProvider_;
    ITaskRunner* taskRunner_;
    UploadManager* uploadManager_;
    CMyEngineList* myEngineList_;
    CLogWindow* logWindow_;
    BasicSettings* settings_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
    std::shared_ptr<UrlShorteningFilter> urlShorteningFilter_;
    TaskDispatcher* taskDispatcher_;
    AbstractServerIconCache* serverIconCache_ = nullptr;
};


ServiceLocator::ServiceLocator() : d_ptr(new ServiceLocatorPrivate()){
}


std::shared_ptr<ILogger> ServiceLocator::logger() {
    return  d_ptr->logger_;
}

void ServiceLocator::setLogger(std::shared_ptr<ILogger> logger) {
    d_ptr->logger_ = logger;
}

ITranslator* ServiceLocator::translator() {
    return d_ptr->translator_;
}

void ServiceLocator::setTranslator(ITranslator* transl) {
    d_ptr->translator_ = transl;
}

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

std::shared_ptr<IUploadErrorHandler> ServiceLocator::uploadErrorHandler() {
    return d_ptr->uploadErrorHandler_;
}

void ServiceLocator::setUploadErrorHandler(std::shared_ptr<IUploadErrorHandler> errorHandler) {
    d_ptr->uploadErrorHandler_ = errorHandler;
}



IDialogProvider* ServiceLocator::dialogProvider() {
    return d_ptr->dialogProvider_;
}

void ServiceLocator::setDialogProvider(IDialogProvider* dialogProvider) {
    d_ptr->dialogProvider_ = dialogProvider;
}

ITaskRunner* ServiceLocator::taskRunner() {
    return d_ptr->taskRunner_;
}

void ServiceLocator::setTaskRunner(ITaskRunner* dispatcher) {
    d_ptr->taskRunner_ = dispatcher;
}

void ServiceLocator::setProgramWindow(IProgramWindow* window) {
    d_ptr->programWindow_ = window;
}


UploadManager* ServiceLocator::uploadManager() const {
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

std::shared_ptr<UrlShorteningFilter> ServiceLocator::urlShorteningFilter() const {
    return d_ptr->urlShorteningFilter_;
}

void ServiceLocator::setUrlShorteningFilter(std::shared_ptr<UrlShorteningFilter> filter) {
    d_ptr->urlShorteningFilter_ = filter;
}

void ServiceLocator::setNetworkClientFactory(std::shared_ptr<INetworkClientFactory> factory) {
    d_ptr->networkClientFactory_ = factory;
}

std::shared_ptr<INetworkClientFactory> ServiceLocator::networkClientFactory() const {
    return d_ptr->networkClientFactory_;
}

void ServiceLocator::setTaskDispatcher(TaskDispatcher* dispatcher) {
    d_ptr->taskDispatcher_ = dispatcher;
}

TaskDispatcher* ServiceLocator::taskDispatcher() const {
    return d_ptr->taskDispatcher_;
}


void ServiceLocator::setServerIconCache(AbstractServerIconCache* cache)
{
    d_ptr->serverIconCache_ = cache;
}


AbstractServerIconCache* ServiceLocator::serverIconCache() const
{
    return d_ptr->serverIconCache_;
}

ServiceLocator* ServiceLocator::instance() {
    static ServiceLocator inst;
    return &inst;
}
