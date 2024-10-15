#pragma once

#include "AbstractParameter.h"

class TextParameter: public AbstractParameter {
public:
    explicit TextParameter(std::string name);

    std::string getType() const override;
    std::string getValue() const override;
    void setValue(const std::string& val) override;

private:
    std::string value_;
};
