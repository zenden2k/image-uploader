#pragma once

#include "AbstractParameter.h"

class TextParameter: public AbstractParameter {
public:
    explicit TextParameter(std::string name);

    std::string getType() const override;
    std::string getValueAsString() const override;
    void setValue(const std::string& val) override;

    inline static const std::string TYPE = "text";

private:
    std::string value_;
};
