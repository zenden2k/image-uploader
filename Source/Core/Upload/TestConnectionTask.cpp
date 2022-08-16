#include "TestConnectionTask.h"

TestConnectionTask::TestConnectionTask() {

}

UploadTask::Type TestConnectionTask::type() const {
    return TypeTest;
}

std::string TestConnectionTask::getMimeType() const {
    return {};
}

int64_t TestConnectionTask::getDataLength() const {
    return 0;
}

std::string TestConnectionTask::toString() {
    return "TestConnection";
}

std::string TestConnectionTask::title() const {
    return "TestConnection";
}

int TestConnectionTask::retryLimit() {
    return 1;
}
