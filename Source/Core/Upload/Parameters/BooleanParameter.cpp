#include "BooleanParameter.h"

#include "Core/Utils/StringUtils.h"

BooleanParameter::BooleanParameter(std::string name)
    : AbstractParameter(std::move(name))
{
}

std::string BooleanParameter::getType() const {
    return TYPE;
}

std::string BooleanParameter::getValueAsString() const {
    return value_ ? "1" : "0";
}

void BooleanParameter::setValue(const std::string& val) {
    std::string lower = IuStringUtils::ToLower(val);
    if (val == "1" || lower == "true") {
        value_ = true;
    } else {
        value_ = false;
    }
}

void BooleanParameter::setValue(bool val) {
    value_ = val;
}

bool BooleanParameter::getValue() const {
    return value_;
}
