#pragma once

#include "AbstractParameter.h"

class BooleanParameter: public AbstractParameter {
public:
    explicit BooleanParameter(std::string name);

    std::string getType() const override;
    std::string getValueAsString() const override;
    void setValue(const std::string& val) override;

    inline static const std::string TYPE = "boolean";

    void setValue(bool val);
    bool getValue() const;
private:
    bool value_ = false;
};
