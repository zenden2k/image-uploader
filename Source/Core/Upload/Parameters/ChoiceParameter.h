#pragma once

#include "AbstractParameter.h"

class ChoiceParameter: public AbstractParameter {
public:
    explicit ChoiceParameter(std::string name);

    std::string getType() const override;
    std::string getValue() const override;
    void setValue(const std::string& val) override;
    void addItem(const std::string& id, const std::string& value);
    void clearItems();
    const std::vector<std::pair<std::string, std::string>>& getItems() const;

    inline static const std::string TYPE = "choice";
private:
    std::string value_;
    std::vector<std::pair<std::string, std::string>> items_;
};
