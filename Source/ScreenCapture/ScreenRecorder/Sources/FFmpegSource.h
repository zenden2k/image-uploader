#pragma once

#include <string>

class FFmpegInputArgs;
class FFmpegOptions;
class GlobalFFmpegArgs;

class FFmpegSource {

public:
    FFmpegSource(std::string name, bool hidden = false)
        : name_(std::move(name))
        , hidden_(hidden)
    {

    }

    virtual ~FFmpegSource() = default;
    virtual void apply(const FFmpegOptions& settings, FFmpegInputArgs& inputArgs, GlobalFFmpegArgs& args) {
    };

    std::string name() const {
        return name_;
    }

    bool hidden() const {
        return hidden_;
    }

    static std::pair<std::string, std::string> parseSourceId(const std::string& id) {
        std::string sourceId = id;
        std::string param;

        if (id[0] == '[') {
            auto pos = id.find(']');
            if (pos != std::string::npos) {
                sourceId = id.substr(1, pos - 1);
                param = id.substr(pos + 1);
            }
        }
        return { sourceId, param };
    }

private:
    std::string name_;
    bool hidden_;
};
