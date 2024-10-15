#include "ParameterListAdapter.h"

#include "Core/CommonDefs.h"
#include "Func/WinUtils.h"
#include "3rdpart/PropertyList.h"
#include "Core/Upload/Parameters/TextParameter.h"
#include "Core/Upload/Parameters/ChoiceParameter.h"
#include "Core/Upload/UploadEngine.h"

class TextParameterController : public IParameterController {
public:
    HPROPERTY show(AbstractParameter* parameter, CString title, const std::string& value, LPARAM lParam) override {
        auto par = dynamic_cast<TextParameter*>(parameter);
        if (!par) {
            return nullptr;
        }
        return PropCreateSimple(title, U2W(value), lParam);
    }

    std::string save(AbstractParameter* parameter, const CComVariant& value) override {
        return WCstringToUtf8(value.bstrVal);
    }
};

class ChoiceParameterController : public IParameterController {
public:
    HPROPERTY show(AbstractParameter* parameter, CString title, const std::string& value, LPARAM lParam) override {
        auto par = dynamic_cast<ChoiceParameter*>(parameter);
        if (!par) {
            return nullptr;
        }

        const auto& items = par->getItems();
        size_t selectedIndex = 0;
        bool found = false;
        size_t i = 0;
        std::vector<CString> s; // Maybe only this vector of CString is enough?
        std::vector<LPCTSTR> choices;
        for (const auto& it : items) {
            s.push_back(U2W(it.second));
            choices.push_back(s.back().GetString());
            if (!found && it.first == value) {
                selectedIndex = i;
                found = true;
            }
            ++i;
        }
        choices.push_back(nullptr);
        return PropCreateList(U2W(par->getTitle()), choices.data(), selectedIndex, lParam);
    }

    std::string save(AbstractParameter* parameter, const CComVariant& value) override {
        auto choiceParameter = dynamic_cast<ChoiceParameter*>(parameter);
        if (choiceParameter) {
            const auto& items = choiceParameter->getItems();
            try {
                const auto& [id, label] = items.at(value.intVal);
                return id;
            } catch (const std::out_of_range& e) {
            }
        }
        return {};
    }
};

ParameterListAdapter::ParameterListAdapter(ParameterList* parameterList, CPropertyListCtrl* control):
    parameterList_(parameterList),
    control_(control)
{
    registerParameterController(TextParameter::TYPE, std::make_unique<TextParameterController>());
    registerParameterController(ChoiceParameter::TYPE, std::make_unique<ChoiceParameterController>());
}

void ParameterListAdapter::updateControl(ServerSettingsStruct* serverSettings) {
    for (const auto& parameter : *parameterList_) {
        HPROPERTY prop = nullptr;
        CString name = U2W(parameter->getName());
        std::string value = serverSettings ? serverSettings->params[parameter->getName()] : std::string();
        auto lParam = reinterpret_cast<LPARAM>(parameter.get());

        auto controller = getParameterController(parameter->getType());

        if (controller) {
            prop = controller->show(parameter.get(), U2W(parameter->getTitle()), value, lParam);
        }

        if (prop) {
            control_->AddItem(prop);
        }
    }
}

void ParameterListAdapter::saveValues(ServerSettingsStruct* serverSettings) {
    for (const auto& parameter : *parameterList_) {
        HPROPERTY pr = control_->FindProperty(reinterpret_cast<LPARAM>(parameter.get()));

        if (pr) {
            CComVariant vValue;
            pr->GetValue(&vValue);

           auto controller = getParameterController(parameter->getType());
            std::string result;
             
            if (controller) {
                result = controller->save(parameter.get(), vValue);
            }

            serverSettings->params[parameter->getName()] = result;
        }
    }
}

void ParameterListAdapter::registerParameterController(const std::string& type, std::unique_ptr<IParameterController> controller) {
    registeredTypes_[type] = std::move(controller);
}

IParameterController* ParameterListAdapter::getParameterController(const std::string& typeStr) const {
    auto it = registeredTypes_.find(typeStr);

    if (it != registeredTypes_.end()) {
        return it->second.get();
    }

    // Fallback to text parameter type
    it = registeredTypes_.find(TextParameter::TYPE);
    if (it != registeredTypes_.end()) {
        return it->second.get();
    }
    return {};
}

