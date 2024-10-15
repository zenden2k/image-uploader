#include "TextParameter.h"

TextParameter::TextParameter(std::string title)
    : AbstractParameter(std::move(title))
{
}

std::string TextParameter::getType() const {
    return "text";
}

std::string TextParameter::getValue() const {
    return value_;
}

void TextParameter::setValue(const std::string& val) {
    value_ = val;
}
