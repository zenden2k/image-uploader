#pragma once

#include "TextParameter.h"

class FileNameParameter: public TextParameter {
public:
    explicit FileNameParameter(std::string name);

    std::string getType() const override;

    bool directory() const;
    void setDirectory(bool enable);

    inline static const std::string TYPE = "filename";

protected:
    bool directory_ = false;
};
