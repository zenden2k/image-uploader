#include "UrlShorteningTask.h"

#include <Core/Utils/CoreUtils.h>

UrlShorteningTask::UrlShorteningTask(const std::string& url) {
	url_ = url;
}


std::string UrlShorteningTask::getType() const {
	return "url";
}

std::string UrlShorteningTask::getMimeType() const {
	return "text/plain";
}

int64_t UrlShorteningTask::getDataLength() const {
	return url_.length();
}

std::string UrlShorteningTask::getUrl() const {
	return url_;
}