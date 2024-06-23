#include "XmlTemplateGenerator.h"

#include "Core/3rdpart/pcreplusplus.h"
#include "Core/Utils/StringUtils.h"

namespace ImageUploader::Core::OutputGenerator {

XmlTemplateGenerator::XmlTemplateGenerator(XmlTemplateList* templateList): templateList_(templateList) {

}

std::string XmlTemplateGenerator::generate(const std::vector<UploadObject>& items) {
    std::string res;
    std::string itemsStr;
    std::map<std::string, std::string> m_Vars;
    size_t n = items.size();
    int p = itemsPerLine_;
    const auto& templ = templateList_->at(templateIndex_);
    for (int i = 0; i < n; i++) {
        auto& item = items[i];
        if (item.isNull()) {
            continue;
        }
        std::string fname = item.onlyFileName();
        std::string imageUrl = item.getImageUrl(shortenUrl_);
        m_Vars["DownloadUrl"] = item.getDownloadUrl(shortenUrl_);
        m_Vars["ImageUrl"] = imageUrl.empty() ? item.getDownloadUrl(shortenUrl_) : imageUrl;
        m_Vars["ThumbUrl"] = item.getThumbUrl(shortenUrl_);
        m_Vars["FileName"] = fname;
        m_Vars["FullFileName"] = item.displayFileName;
        m_Vars["Index"] = std::to_string(i);
        //CString buffer = WinUtils::GetOnlyFileName(UrlList[i].FileName);
        m_Vars["FileNameWithoutExt"] = IuCoreUtils::ExtractFileNameNoExt(item.displayFileName);
        if (p != 0 && !((i) % p))

            itemsStr += replaceVars(templ.LineStart, m_Vars);
        itemsStr += replaceVars(templ.Items, m_Vars);
        if ((p != 0 && ((i + 1) % p)) || p == 0)
            itemsStr += replaceVars(templ.ItemSep, m_Vars);

        if (p != 0 && !((i + 1) % p)) {
            itemsStr += replaceVars(templ.LineEnd, m_Vars);
            if (p != 0)
                itemsStr += replaceVars(templ.LineSep, m_Vars);
        }
    }
    m_Vars.clear();
    m_Vars["Items"] = itemsStr;
    res += replaceVars(templ.TemplateText, m_Vars);
    return res;
}

GeneratorID XmlTemplateGenerator::id() const
{
    return gidXmlTemplate;
}

std::string XmlTemplateGenerator::replaceVars(const std::string& text, const std::map<std::string, std::string>& m_Vars) {
    std::string Result = text;

    pcrepp::Pcre reg("\\$\\(([A-z0-9_]*?)\\)", "imc");
    std::string str = text;
    size_t pos = 0;
    while (pos <= str.length()) {
        if (reg.search(str, pos)) {
            pos = reg.get_match_end() + 1;
            std::string vv = reg[1];
            Result = IuStringUtils::Replace(Result, "$(" + vv + ")", m_Vars.at(vv));
        }
        else {
            break;
        }
    }
    Result = IuStringUtils::Replace(Result, "\\n", "\r\n");

    return Result;
}

void XmlTemplateGenerator::setTemplateIndex(size_t index) {
    templateIndex_ = index;
}

}
