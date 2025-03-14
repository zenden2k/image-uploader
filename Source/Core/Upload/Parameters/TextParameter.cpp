#include "TextParameter.h"

TextParameter::TextParameter(std::string title)
    : AbstractParameter(std::move(title))
{
}

std::string TextParameter::getType() const {
    return TYPE;
}

std::string TextParameter::getValueAsString() const {
    return value_;
}

void TextParameter::setValue(const std::string& val) {
    value_ = val;
}
