#include "FFmpegInputArgs.h"

FFmpegInputArgs::FFmpegInputArgs(std::string file): input_(std::move(file)) {
}

FFmpegInputArgs& FFmpegInputArgs::setVideoSize(int width, int height) {
    return addArg("video_size", std::to_string(width) + "x" + std::to_string(height));
}

FFmpegInputArgs& FFmpegInputArgs::setFrameRate(int FrameRate) {
    return addArg("r", FrameRate);
}

FFmpegInputArgs& FFmpegInputArgs::setFormat(const std::string& Format) {
    return addArg("f", Format);
}

FFmpegInputArgs& FFmpegInputArgs::setAudioCodec(const std::string& Codec) {
    return addArg("acodec", Codec);
}

FFmpegInputArgs& FFmpegInputArgs::setAudioFrequency(int Frequency) {
    return addArg("ar", Frequency);
}

FFmpegInputArgs& FFmpegInputArgs::setAudioChannels(int Channels) {
    return addArg("ac", Channels);
}

FFmpegInputArgs& FFmpegInputArgs::disableVideo() {
    return addArg("-vn");
}

void FFmpegInputArgs::setSourceApplied(const std::string& sourceId) {
    appliedSources_.insert(sourceId);
}

bool FFmpegInputArgs::isSourceApplied(const std::string& sourceId) const {
    return appliedSources_.find(sourceId) != appliedSources_.end();
}
