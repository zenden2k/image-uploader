#include "ParameterFactory.h"


#include <unordered_map>
#include <functional>

#include "Core/Scripting/API/ScriptAPI.h"
#include "TextParameter.h"
#include "ChoiceParameter.h"
#include "BooleanParameter.h"

class ParameterFactory {
public:
    virtual std::unique_ptr<AbstractParameter> create(const std::string& name, Sqrat::Table& table) const = 0;
};

class ChoiceParameterFactory : public ParameterFactory {
public:
    std::unique_ptr<AbstractParameter> create(const std::string& name, Sqrat::Table& table) const override {
        auto choiceParameter = std::make_unique<ChoiceParameter>(name);
        auto choices = table.GetValue<Sqrat::Array>("items");
        if (!!choices) {
            Sqrat::Array::iterator it;
            while (choices->Next(it)) {
                Sqrat::Table tbl(it.getValue(), choices->GetVM());
                std::string itemLabel = ScriptAPI::GetValue(tbl.GetValue<std::string>("label"));
                std::string itemId = ScriptAPI::GetValue(tbl.GetValue<std::string>("id"));
                choiceParameter->addItem(itemId, itemLabel);
            }
        }
        return choiceParameter;
    }
};

class BooleanParameterFactory : public ParameterFactory {
public:
    std::unique_ptr<AbstractParameter> create(const std::string& name, Sqrat::Table& table) const override {
        return std::make_unique<BooleanParameter>(name);
    }
};

class TextParameterFactory : public ParameterFactory {
public:
    std::unique_ptr<AbstractParameter> create(const std::string& name, Sqrat::Table& table) const override {
        return std::make_unique<TextParameter>(name);
    }
};

class ParameterFactoryRegistry {
public:
    ParameterFactoryRegistry() {
        // Registering factories by parameter type
        registry_[ChoiceParameter::TYPE] = std::make_unique<ChoiceParameterFactory>();
        registry_[BooleanParameter::TYPE] = std::make_unique<BooleanParameterFactory>();
        registry_[TextParameter::TYPE] = std::make_unique<TextParameterFactory>();  // TextParameter by default
    }

    std::unique_ptr<AbstractParameter> createParameter(const std::string& type, const std::string& name, Sqrat::Table& table) const {
        auto it = registry_.find(type);
        if (it != registry_.end()) {
            return it->second->create(name, table);
        }
        // Fallback to default factory
        it = registry_.find(TextParameter::TYPE);
        if (it != registry_.end()) {
            return it->second->create(name, table);
        }
        return {};
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ParameterFactory>> registry_;
};

std::unique_ptr<AbstractParameter> SqTableToParameter(const std::string& name, const std::string& type, Sqrat::Table& table) {
    static ParameterFactoryRegistry factoryRegistry;
    return factoryRegistry.createParameter(type, name, table);
}
