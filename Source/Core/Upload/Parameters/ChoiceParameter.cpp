#include "ChoiceParameter.h"

ChoiceParameter::ChoiceParameter(std::string title)
    : AbstractParameter(std::move(title))
{
}

std::string ChoiceParameter::getType() const {
    return TYPE;
}

std::string ChoiceParameter::getValue() const {
    return value_;
}

void ChoiceParameter::setValue(const std::string& val) {
    value_ = val;
}

void ChoiceParameter::addItem(const std::string& id, const std::string& value) {
    items_.push_back({ id, value });
}

void ChoiceParameter::clearItems() {
    items_.clear();
}

const std::vector<std::pair<std::string, std::string>>& ChoiceParameter::getItems() const{
    return items_;
}
