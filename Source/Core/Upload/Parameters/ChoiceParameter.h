#pragma once

#include "AbstractParameter.h"

class ChoiceParameter: public AbstractParameter {
public:
    explicit ChoiceParameter(std::string name);

    std::string getType() const override;
    std::string getValueAsString() const override;
    void setValue(const std::string& val) override;
    void addItem(const std::string& id, const std::string& value);
    void clearItems();
    const std::vector<std::pair<std::string, std::string>>& getItems() const;

    inline static const std::string TYPE = "choice";
    void setSelectedIndex(int val);
    int selectedIndex() const;

    std::string getDescription() const override;

private:
    std::vector<std::pair<std::string, std::string>> items_;
    int selectedIndex_ = -1;
};
